# maiken (`mkn`)

**A YAML-driven, cross-platform build tool for C, C++, Obj-C, CUDA, OpenCL and C#.**

[![Build](https://github.com/mkn/mkn/actions/workflows/build.yml/badge.svg)](https://github.com/mkn/mkn/actions/workflows/build.yml)
[![Release](https://img.shields.io/badge/binaries-latest-blue)](https://github.com/mkn/mkn/releases/tag/latest)
[![License: BSD](https://img.shields.io/badge/license-BSD-green)](LICENSE.md)

One `mkn.yaml` per project. Dependencies are fetched from git and built for you,
profiles cover debug/test/platform variants, and the same file works with gcc, clang,
MSVC, nvcc and icc on Linux, macOS, BSD and Windows.

```yaml
name: hello
main: hello.cpp
```

```sh
mkn build run
```

---

## Contents

- [Install](#install)
- [Quick start](#quick-start)
- [The mkn.yaml file](#the-mknyaml-file)
- [Everyday commands](#everyday-commands)
- [Dependencies](#dependencies)
- [Modules](#modules)
- [Configuration](#configuration)
- [Building maiken from source](#building-maiken-from-source)
- [Full manual](#full-manual)
- [Contributing](#contributing)

---

## Install

Prebuilt binaries are published on the
[latest release](https://github.com/mkn/mkn/releases/tag/latest):

| Platform              | Asset                |
|-----------------------|----------------------|
| Linux x86_64 (glibc, portable) | `mkn_manylinux`     |
| Linux arm64 (portable)| `mkn_manylinux_arm`  |
| Linux x86_64 (Ubuntu) | `mkn_nix`            |
| Linux arm64 (Ubuntu)  | `mkn_arm_linux`      |
| macOS arm64           | `mkn_arm_osx`        |
| Windows x86_64        | `mkn.exe`            |

```sh
curl -Lo mkn https://github.com/mkn/mkn/releases/download/latest/mkn_manylinux
chmod +x mkn && mv mkn ~/.local/bin/   # or anywhere on PATH
mkn -v
```

A C/C++ compiler and `git` must be on `PATH`. On first run, maiken detects your
compiler and writes a default [`settings.yaml`](#configuration).

## Quick start

```console
$ mkdir hello && cd hello
$ mkn init
$ cat mkn.yaml
name: hello_world
#inc: ./inc
#src: ./src
main: cpp.cpp
$ mkn build
Project: ~/hello
Creating bin: ~/hello/bin/build/hello_world
BUILD TIME: 1073 ms
FINISHED:   2026-09-26-13:06:26
$ mkn run
HELLO WORLD!
```

`mkn init` writes `mkn.yaml` and a hello world `cpp.cpp`. Output goes to `bin/<profile>`,
`bin/build` when no profile is selected.

Add `-t` to compile in parallel, `-O` to optimise, `-g` for debug symbols:

```sh
mkn clean build -tO run
```

## The mkn.yaml file

**Executable** — a `main` makes it a binary:

```yaml
name: app
main: main.cpp
src: src
inc: inc
```

**Library** — no `main` means a library:

```yaml
name: mylib
inc: inc
src: src
```

**Profiles** — named variants that can inherit from each other with `parent`:

```yaml
#! clean build test -dtOp test   # default command when running plain `mkn`

name: mylib
version: master

profile:
- name: base
  inc: inc
  src: src
  dep: mkn.kul

- name: test
  parent: base
  test: test/(\w*).cpp     # regex; each match is built and run by `mkn test`

- name: bench
  parent: base
  main: bench/bench.cpp
  arg: -DNDEBUG
```

Select one with `mkn build -p test`. Platform-specific settings go under
`if_arg`, `if_src`, `if_inc`, `if_lib` keyed by `nix`, `bsd`, `win`
(and `nix_shared`, `win_static`, …).

Properties can be defined and referenced with `${...}`, and built-ins such as
`${OS}`, `${ARCH}`, `${MKN_ROOT}` are always available.

See the [mkn.yaml manual](doc/mkn/man/yaml.md) for the full schema.

## Everyday commands

| Command        | Does                                                        |
|----------------|-------------------------------------------------------------|
| `mkn init`     | Create a minimal `mkn.yaml`                                 |
| `mkn build`    | Compile and link all active projects                        |
| `mkn clean`    | Delete `./bin/<profile>`                                    |
| `mkn run`      | Run the binary, with dependency library paths set up        |
| `mkn dbg`      | Run the binary under gdb / lldb / cdb (`MKN_DBG` overrides) |
| `mkn test`     | Build and run each file listed under the profile's `test:`  |
| `mkn profiles` | List the profiles in `./mkn.yaml`                           |
| `mkn tree`     | Print the dependency tree                                   |
| `mkn info`     | Show repository, threads and compilers in use               |
| `mkn pack`     | Collect binaries and libraries into `bin/<profile>/pack`    |

Common flags (combine freely, e.g. `mkn clean build -dtKOp test`):

| Flag              | Meaning                                                   |
|-------------------|-----------------------------------------------------------|
| `-p <profile>`    | Activate a profile                                        |
| `-d [n\|csv]`     | Also build dependencies (all, `n` levels, or named ones)  |
| `-t [n]`          | Parallel compile (`n` threads, default: optimal)          |
| `-O [0-9]` `-g [0-9]` `-W [0-9]` | Optimisation / debug / warning level       |
| `-K` / `-S`       | Link static / shared                                      |
| `-a "<args>"`     | Extra compiler args (or program args with `run`)          |
| `-l "<args>"`     | Extra linker args                                         |
| `-w <dep>` / `-T <dep>` | Add / remove a dependency from the command line     |
| `-R`              | Dry run: print commands without executing                 |
| `-x <file>`       | Use another `settings.yaml`                               |

`mkn -h` prints everything.

## Dependencies

List dependencies by name; maiken clones and builds them into its local repository
(`~/.maiken/repo` by default):

```yaml
dep:
  - name: mkn.kul
  - name: parse.yaml
    version: master
    profile: shared
```

Names without a URL are resolved against `MKN_REMOTE_REPO`
(default `https://github.com/mkn/`). A dependency can also be pulled in just for one build:

```sh
mkn build -w mkn.ram[https]
mkn build -w https://github.com/mkn/mkn.ram[https]
```

Each `-w` dependency adds a `-DMKN_WITH_<NAME>` define, e.g. `-DMKN_WITH_MKN_RAM`.

## Modules

Modules are plugins that hook into the build phases (init / compile / link / pack),
declared under `mod:` or added with `-m`. Official modules live at
[github.com/mkn-mod](https://github.com/mkn-mod), including:

| Module             | Purpose                                     |
|--------------------|---------------------------------------------|
| [clang.format](https://github.com/mkn-mod/clang.format) | Run clang-format over sources before linking |
| [clang.tidy](https://github.com/mkn-mod/clang.tidy) | Run clang-tidy                  |
| [conan.install](https://github.com/mkn-mod/conan.install) | Fetch Conan package binaries        |
| [lang.python3](https://github.com/mkn-mod/lang.python3) | Compile/link against Python 3 |
| [lang.swig](https://github.com/mkn-mod/lang.swig) | Generate SWIG bindings at compile time |
| [subl.lsp-clangd](https://github.com/mkn-mod/subl.lsp-clangd) | Generate Sublime Text LSP clangd config |

```yaml
profile:
- name: format
  mod: clang.format{init{style: file, paths: inc src test}}
```

```sh
mkn build -m clang.format{init{style:file,paths:src}}   # or ad hoc
```

## Configuration

**`settings.yaml`** holds global defaults: compilers per file type, extra include/library
paths, local repository location and compiler masks.
See [settings.yaml](doc/mkn/man/settings.md) for its schema and location.

**Logging**: set `KLOG` to see what maiken is doing:

```sh
KLOG=3 mkn build      # 1=INFO 2=ERR 3=DBG 4=OTH 5=TRACE
mkn build -V 3        # same, as an argument
```

Other environment variables and compile-time switches are listed in the
manual: [environment variables](doc/mkn/man/env.md) and [switches](doc/mkn/man/switches.md).

## Building maiken from source

Requires a C++20 compiler and git 2.0+.

**Linux / macOS / BSD**

```sh
git clone https://github.com/mkn/mkn maiken/master && cd maiken/master
./res/ci/nixish_setup.sh   # fetches mkn.kul and yaml-cpp into ./ext
make nix                   # macOS / BSD: make bsd CXX=clang++
./mkn build -dtKO          # rebuild maiken with itself
```

**Windows** (from a shell where `vcvarsall.bat` has been run, e.g. Git Bash):

```sh
git clone https://github.com/mkn/mkn maiken/master && cd maiken/master
./res/ci/win_build.sh      # CC=clang to use clang instead of cl
```

## Full manual

[doc/mkn/man](doc/mkn/man/README.md) is the reference manual: full YAML schema, profile
inheritance rules, `settings.yaml` schema, dependency resolution, environment variables
and switches.

## Contributing

Bug reports, questions and feature requests are welcome via
[issues](https://github.com/mkn/mkn/issues).

Licensed under the [BSD license](LICENSE.md).
