# Building maiken

The bootstrap scripts clone the dependencies [mkn.kul](https://github.com/mkn/mkn.kul) and
[parse.yaml](https://github.com/mkn/parse.yaml) (with yaml-cpp) into `./ext`, then compile `./mkn`
without needing an existing maiken. The resulting binary can then rebuild maiken with itself.

Example settings files for various toolchains can be found in [res/mkn](../../../res/mkn).

Platform specific compiler arguments are listed in [Platforms](../os.md).

## Modules

Module support is opt-in and is not compiled into the bootstrap binary. To use modules,
rebuild maiken with itself adding `-w mkn.mod`, e.g.

```sh
./mkn clean build -dtKOWa "-std=c++20" -w mkn.mod
```

## Windows

Prerequisites:

- git 2.0
- C++20 compiler (MSVC or clang)
- Windows SDK 10+
- A bash shell (e.g. Git Bash) with the MSVC environment loaded (`vcvarsall.bat amd64`)

Operations:

```sh
git clone https://github.com/mkn/mkn maiken/master
cd maiken/master
./res/ci/win_build.sh            # MSVC
CC=clang ./res/ci/win_build.sh   # clang
```

`/bin/link` from Git Bash can shadow MSVC's `link.exe`, remove or rename it if linking fails.

Testing (from cmd):

```bat
SET MKN_CL_PREFERRED=1
mkn build -dtKO 2 -g 0 -a "-std:c++20 -EHsc -DYAML_CPP_STATIC_DEFINE"
```

## Unix

Prerequisites:

- git 2.0
- C++20 compiler (GCC/Clang)

### Linux

Operations:

```sh
git clone https://github.com/mkn/mkn maiken/master
cd maiken/master
./res/ci/nixish_setup.sh
make nix                               # shared
make nix LDFLAGS="-pthread -ldl"       # static
make nix CXX=clang++                   # with clang
```

If clang is secondary to gcc:

```sh
GCC_VER=$(gcc --version | grep ^gcc | sed 's/^.* //g')
CXXFLAGS="-O2 -std=c++20 -Wall -I<GCC_INSTALL>/include/c++/${GCC_VER} -I<GCC_INSTALL>/include/c++/${GCC_VER}/x86_64-unknown-linux-gnu"
LDFLAGS="-L<GCC_INSTALL>/lib64"
LD_LIBRARY_PATH=<GCC_INSTALL>/lib64 make nix CXX=clang++ CXXFLAGS="${CXXFLAGS}" LDFLAGS="${LDFLAGS}"
```

Testing:

```sh
./mkn clean build -dtKOWa "-std=c++20"
```

### Mac/OSX

Operations:

```sh
git clone https://github.com/mkn/mkn maiken/master
cd maiken/master
./res/ci/nixish_setup.sh
make bsd CXX=clang++
```

Testing:

```sh
./mkn clean build -dtKOWa "-std=c++20"
```

### NetBSD/etc

Operations:

```sh
git clone https://github.com/mkn/mkn maiken/master
cd maiken/master
./res/ci/nixish_setup.sh
gmake bsd LDFLAGS="-pthread -ldl -lexecinfo"
```

Testing:

```sh
./mkn clean build -dtKOWa "-std=c++20" -l "-pthread -ldl -lexecinfo"
```

### FreeBSD

Operations:

```sh
git clone https://github.com/mkn/mkn maiken/master
cd maiken/master
./res/ci/nixish_setup.sh
gmake bsd CXXFLAGS="-std=c++20 -Os -Wall -D_GLIBCXX_USE_C99"
```

Testing:

```sh
./mkn clean build -dtKOWa "-std=c++20 -D_GLIBCXX_USE_C99" -l "-pthread -ldl"
```
