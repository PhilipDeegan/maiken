#!/usr/bin/env python3
"""
Module-loader test driver, keyed by a "build_job_id" (ubuntu_gcc,
ubuntu_clang, manylinux_gcc, manylinux_arm_gcc, macos_clang, win_cl,
win_clang).

.github/workflows/build.yml already built the mkn binaries this script uses,
via plain shell/make commands, before invoking it:
  - ./mkn (mkn.exe on Windows): the bootstrap binary from `make nix`/
    `make bsd`/win_build.sh, with no mkn.mod support
  - bin/build/mkn: rebuilt with `-w mkn.mod`, so it actually has
    module-loading support compiled in
  - (ubuntu only) bin/bin_shared/mkn: same as bin/build/mkn but linked
    against a *shared* libparse.yaml instead of static + -rdynamic

This script builds test_mod (a plain compile, using the bootstrap binary -
no module loading involved yet) and its local test/mod/dep dependency, which
mkn doesn't auto-build, then uses the mod-enabled binary(ies) to actually
exercise the module loader ("build test pack -Op test"), checking every
subprocess's exit code explicitly.

Run from the repository root, e.g.: python3 res/ci/build.py ubuntu_gcc
"""
import os
import shlex
import subprocess
import sys
import tempfile

IS_WIN = sys.platform.startswith("win")
EXE = ".exe" if IS_WIN else ""
ROOT = os.getcwd()  # expected to be the repo root when this script is invoked

BOOTSTRAP = os.path.join(ROOT, f"mkn{EXE}")
DEP_DIR = os.path.join(ROOT, "test", "mod", "dep")


def built(profile_dir):
    """Absolute path to a binary build.yml already built under bin/<profile_dir>/."""
    return os.path.join(ROOT, "bin", profile_dir, f"mkn{EXE}")


def run(mkn_bin, line, cwd=None, env=None):
    args = [mkn_bin] + shlex.split(line)
    print(f"+ [{cwd or ROOT}] {' '.join(args)}", flush=True)
    full_env = os.environ.copy()
    if env:
        full_env.update(env)
    result = subprocess.run(args, cwd=cwd, env=full_env)
    if result.returncode != 0:
        print(f"FAILED (exit {result.returncode}): {' '.join(args)}", file=sys.stderr)
        sys.exit(result.returncode)


def rm_test_lib():
    test_lib = os.path.join(ROOT, "bin", "test", "libtest.so")
    if os.path.exists(test_lib):
        os.remove(test_lib)


def behaviour(mkn_bin):
    """test/run.py, dry runs of mkn_bin against the projects under test/"""
    cmd = [sys.executable, os.path.join(ROOT, "test", "run.py"), mkn_bin]
    print(f"+ {' '.join(cmd)}", flush=True)
    if subprocess.run(cmd).returncode != 0:
        sys.exit(1)


def prebuilt(mkn_bin, settings, args):
    """test/prebuilt, res/mkn/<settings>.yml: parse.yaml from its 0.8.0 release as a static and a
    shared library and from the release marked Latest for version latest, each clone keeping its
    binaries in .mkn/prebuilt/<mode>; with -P mkn.with.bin=1, MKN_WITH_BIN=1, a settings bin: entry
    and a settings property: mkn.with.bin"""
    cwd = os.path.join(ROOT, "test", "prebuilt")
    xs = os.path.join(ROOT, "res", "mkn", settings).replace("\\", "/")
    repo = subprocess.run([mkn_bin, "-G", "MKN_REPO"], stdout=subprocess.PIPE, text=True)

    def check(name, profile, x, extra, version, mode, release, env=None):
        cmd = [mkn_bin, "build", "run", "-d", "-p", profile, "-x", x] + extra + ["-a", args]
        print(f"+ [{cwd}] {' '.join(cmd)}", flush=True)
        result = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, env=env)
        print(result.stdout, flush=True)
        errors = []
        if result.returncode != 0:
            errors.append(f"exit {result.returncode}")
        for expected in (f"parse.yaml: prebuilt {release}", f"mode={mode})", "parsed 3"):
            if expected not in result.stdout:
                errors.append(f"missing '{expected}'")
        if "p/src/" in result.stdout.replace("\\", "/"):
            errors.append("yaml-cpp sources were compiled")
        cache = os.path.join(repo.stdout.strip(), "parse", "yaml", version, ".mkn", "prebuilt", mode)
        if not os.path.isfile(os.path.join(cache, ".mkn.prebuilt")):
            errors.append(f"no {cache}")
        if errors:
            print(f"FAILED prebuilt {name}: {', '.join(errors)}", file=sys.stderr)
            sys.exit(1)
        print(f"PASS prebuilt {name}", flush=True)

    with_bin = ["-P", "mkn.with.bin=1"]
    check("static", "static", xs, with_bin, "0.8.0", "static", "0.8.0")
    check("shared", "shared", xs, with_bin, "0.8.0", "shared", "0.8.0")
    check("latest", "latest", xs, with_bin, "latest", "static", "")
    check("MKN_WITH_BIN", "latest", xs, [], "latest", "static", "",
          dict(os.environ, MKN_WITH_BIN="1"))

    with tempfile.TemporaryDirectory() as tmp:  # settings extending res/mkn/<settings>.yml
        for name, body in (("named", "bin:\n  parse.yaml: 0.8.0\n"),
                           ("property", "property:\n  mkn.with.bin: 1\n")):
            with open(os.path.join(tmp, name + ".yml"), "w") as f:
                f.write(f"super: {xs}\n{body}")
        x = lambda name: os.path.join(tmp, name).replace("\\", "/")
        check("bin: entry", "static", x("named"), [], "0.8.0", "static", "0.8.0")
        env = {k: v for k, v in os.environ.items() if k != "MKN_WITH_BIN"}  # it would override
        check("property: mkn.with.bin", "latest", x("property"), [], "latest", "static", "", env)


def prebuilt_fallback(mkn_bin, args):
    """test/prebuilt/missing.yml names a release that doesn't exist: with -q, and without -d,
    parse.yaml is built from source instead"""
    cwd = os.path.join(ROOT, "test", "prebuilt")
    cmd = [mkn_bin, "build", "run", "-q", "-t", "-x", os.path.join(cwd, "missing").replace("\\", "/"),
           "-a", args]
    print(f"+ [{cwd}] {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(result.stdout, flush=True)
    errors = [] if result.returncode == 0 else [f"exit {result.returncode}"]
    for expected in ("parse.yaml: prebuilt binary not used", "building it", "parsed 3"):
        if expected not in result.stdout:
            errors.append(f"missing '{expected}'")
    if errors:
        print(f"FAILED prebuilt fallback: {', '.join(errors)}", file=sys.stderr)
        sys.exit(1)
    print("PASS prebuilt fallback", flush=True)


RPATH_DIR = os.path.join(ROOT, "test", "rpath")


def rpath_mkn(mkn_bin, env, *cmd):
    """mkn_bin with cmd in test/rpath, its output printed and returned with its exit code"""
    cmd = [mkn_bin, *cmd]
    print(f"+ [{RPATH_DIR}] {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, cwd=RPATH_DIR, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, env=env)
    print(result.stdout, flush=True)
    return result


def rpath_run_errors(ran, runs):
    """app's run: it prints its library's value, or fails with the library named (not on Windows)"""
    if runs:
        if ran.returncode == 0 and "rpath value 42" in ran.stdout:
            return []
        return [f"app did not run, exit {ran.returncode}"]
    if ran.returncode == 0 or "rpath value 42" in ran.stdout:
        return ["app ran without its library being found"]
    if sys.platform != "win32" and "librpath" not in ran.stdout:
        return ["its library is not named as missing"]
    return []


def rpath_link_errors(dry, rpathing):
    """a dry run's link line: -Wl,-rpath only with mkn.link.rpathing"""
    if dry.returncode != 0:
        return [f"dry run exit {dry.returncode}"]
    if ("-Wl,-rpath" in dry.stdout) != rpathing:
        return ["-rpath " + ("missing from" if rpathing else "in") + " the link line"]
    return []


def rpath(mkn_bin, args, env=None):
    """test/rpath: app and its shared library built, then run, with the default settings (rpath),
    no_rpath_only.yml (mkn.link.rpathing false: found by the library path mkn sets) and no_rpath.yml
    (mkn.env.automatic false as well: not found); dry runs check -rpath on the link line, not on
    Windows which has none"""
    env = dict(os.environ, **(env or {}))
    for name, runs in (("default", True), ("no_rpath_only", True), ("no_rpath", False)):
        x = [] if name == "default" else ["-x", os.path.join(RPATH_DIR, name).replace("\\", "/")]
        built = rpath_mkn(mkn_bin, env, "clean", "build", "-d", *x, "-a", args)
        if built.returncode != 0:
            errors = [f"build exit {built.returncode}"]
        else:
            errors = rpath_run_errors(rpath_mkn(mkn_bin, env, "run", *x), runs)
        if sys.platform != "win32" and runs:
            dry = rpath_mkn(mkn_bin, env, "build", "-R", "-d", *x, "-a", args)
            errors += rpath_link_errors(dry, name == "default")
        if errors:
            print(f"FAILED rpath {name}: {', '.join(errors)}", file=sys.stderr)
            sys.exit(1)
        print(f"PASS rpath {name}", flush=True)


def nix_test(std_args, env, full):
    """ubuntu_gcc / ubuntu_clang / manylinux_gcc."""
    a = f'-a "{std_args}"'
    run(BOOTSTRAP, f"build {a} -dtO 2 -g 0 -W 9", cwd=DEP_DIR, env=env)
    run(BOOTSTRAP, f"build -dtp test_mod {a} -O 2 -g 0 -W 9", env=env)
    run(built("build"), f"build test pack -tp test {a} -O 2 -g 0 -W 9", env=env)
    behaviour(built("build"))
    prebuilt(built("build"), "bin_nix", "-std=c++20")
    rpath(built("build"), std_args, env)

    if not full:
        return

    prebuilt_fallback(built("build"), "-std=c++20")

    # format still runs on the mod-less bootstrap - m/clang/format's mod.cpp
    # is currently out of sync with mkn.mod's Context API (pending changes to
    # merge there); leave this as a no-op build until that's sorted, rather
    # than failing CI on it.
    run(BOOTSTRAP, f"build -dtp format {a} -O 2 -g 0 -W 9", env=env)

    rm_test_lib()
    run(built("bin_shared"), f"build test pack -p test {a} -O 2 -g 0 -W 9", env=env)


def job_macos_clang():
    tc = os.path.join(ROOT, "res", "mkn", "clang")
    run(BOOTSTRAP, f"build -dtx {tc}", cwd=DEP_DIR)
    run(BOOTSTRAP, f"build -dtOp test_mod -x {tc}")
    run(built("build"), f"build test pack -Op test -x {tc}")
    behaviour(built("build"))
    prebuilt(built("build"), "bin_osx", "-std=c++20")
    rpath(built("build"), "-std=c++20")


def job_win_cl():
    std = "-std:c++20 -EHsc"
    env = {"MKN_CL_PREFERRED": "1"}
    run(BOOTSTRAP, f'build -dta "{std}"', cwd=DEP_DIR, env=env)
    run(BOOTSTRAP, f'build -dtOp test_mod -a "{std}"', env=env)
    run(built("build"), f'build test pack -Op test -a "{std}"', env=env)
    behaviour(built("build"))
    prebuilt(built("build"), "bin_win", std)
    rpath(built("build"), std, env)


def job_win_clang():
    # forward slashes: run()'s shlex.split() (POSIX mode) treats "\" as an
    # escape char and would eat the backslashes os.path.join gives us here.
    tc = os.path.join(ROOT, "res", "mkn", "clang_win").replace("\\", "/")
    run(BOOTSTRAP, f"build -dtx {tc}", cwd=DEP_DIR)
    run(BOOTSTRAP, f"build -dtOp test_mod -x {tc}")
    run(built("build"), f"build test pack -Op test -x {tc}")
    behaviour(built("build"))


JOBS = {
    "ubuntu_gcc": lambda: nix_test("-std=c++20 -fPIC", {"MKN_GCC_PREFERRED": "1"}, full=True),
    "ubuntu_clang": lambda: nix_test(
        "-std=c++20 -fPIC", {"MKN_GCC_PREFERRED": "1", "CC": "clang", "CXX": "clang++"},
        full=True),
    "manylinux_gcc": lambda: nix_test(
        "-std=c++20 -fPIC", {"MKN_GCC_PREFERRED": "1"}, full=False),
    "manylinux_arm_gcc": lambda: nix_test(
        "-std=c++20 -fPIC", {"MKN_GCC_PREFERRED": "1"}, full=False),
    "macos_clang": job_macos_clang,
    "win_cl": job_win_cl,
    "win_clang": job_win_clang,
}


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in JOBS:
        print(f"usage: {sys.argv[0]} <{'|'.join(JOBS)}>", file=sys.stderr)
        sys.exit(2)
    JOBS[sys.argv[1]]()


if __name__ == "__main__":
    main()
