#! /usr/bin/env python3
"""
Behaviour tests: a built mkn against the projects under test/, as dry runs ("build -R", never with
clean, which a dry run still does) so no compiler runs. Each case checks the exit code and text
expected, or not, in the output.

    python3 test/run.py <path to mkn>

Add a case to CASES, with a project directory under test/ when none fits.
"""

import os
import subprocess
import sys

TEST = os.path.dirname(os.path.abspath(__file__))
APPLE = sys.platform == "darwin"


def source(profile, settings, ok, has=(), lacks=()):
    """test/source: a src: entry's files without a compiler in the settings file list"""
    return dict(
        name=f"source {profile} {settings}",
        cwd=os.path.join(TEST, "source"),
        args=["build", "-R", "-p", profile, "-x", os.path.join(TEST, "source", settings)],
        ok=ok,
        has=has,
        lacks=lacks,
    )


CASES = [
    # a directory: a known source type without a compiler is an error, hidden directories unread
    source(
        "dir",
        "c.yaml",
        ok=False,
        has=["Unable to handle sources in", "asm.S", "1 with types not in file list, of 2"],
        lacks=["hidden.S"],
    ),
    source("dir", "c_S.yaml", ok=True, has=["asm.S", "two.c"], lacks=["hidden.S"]),
    # a file named in src: without a compiler is an error
    source("named", "c.yaml", ok=False, has=["Unable to handle source:", "named.S"]),
    source("named", "c_S.yaml", ok=True, has=["named.S", "two.c"]),
    # files of no source type are skipped
    source("headers", "c.yaml", ok=True, has=["two.c"], lacks=["README"]),
    # C# is a known source type, csc builds it
    source("csharp", "c.yaml", ok=False, has=["Unable to handle sources in", "app.cs"]),
    # Objective-C is a known source type only on Apple
    source("objc", "c.yaml", ok=not APPLE, has=["apple.m"] if APPLE else ["two.c"]),
]


def run(mkn, case):
    result = subprocess.run(
        [mkn] + case["args"],
        cwd=case["cwd"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    out = result.stdout
    errors = []
    if (result.returncode == 0) != case["ok"]:
        errors.append(f"exit {result.returncode}, expected {'0' if case['ok'] else 'failure'}")
    errors += [f"missing '{s}'" for s in case["has"] if s not in out]
    errors += [f"unexpected '{s}'" for s in case["lacks"] if s in out]
    return errors, out


def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <path to mkn>", file=sys.stderr)
        sys.exit(2)
    mkn = os.path.abspath(sys.argv[1])
    failed = 0
    for case in CASES:
        errors, out = run(mkn, case)
        if errors:
            failed += 1
            print(f"FAIL {case['name']}: {', '.join(errors)}\n{out}")
        else:
            print(f"PASS {case['name']}")
    print(f"{len(CASES) - failed}/{len(CASES)} passed")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
