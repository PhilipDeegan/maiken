
# maiken

**Cross platform build tool for C/C++/Obj-C/C#/CUDA/opencl**

## Building maiken prerequisites
  Linux/BSD: GCC 8.0 (or CLANG equivalent) / git 2.0

  Windows  : Visual C++ 2019 / Windows SDK 10+ / git 2.0

[README](https://raw.githubusercontent.com/mkn/mkn/master/README.noformat)

[Binaries available](https://github.com/mkn/mkn/releases/tag/latest)

Testers/comments/suggestions/feature requests welcome. Email me or make an issue. Or head on over to [/r/mkn](http://reddit.com/r/mkn)

**Note: Older compilers may be used by maiken**

## Dependencies

**Automatically acquired by git in Makefile/make.bat**

[mkn.kul](https://github.com/mkn/mkn.kul)

[yaml.cpp](https://github.com/mkn/parse.yaml)

## How to use

mkn init

mkn build

mkn run


## Screenshots

GCC Debian

![GCC Debian](https://raw.githubusercontent.com/PhilipDeegan/maiken/wiki/mkn_nix.png)

MSVC Windows

![MSVC Windows](https://raw.githubusercontent.com/PhilipDeegan/maiken/wiki/mkn_win.png)

Init example

![MSVC Windows](https://raw.githubusercontent.com/PhilipDeegan/maiken/wiki/mkn_init.png)


## Official plugins

**[conan.io](https://github.com/mkn-mod/conan.install)**

  Conan.io package binary retrieval be used when needed.
  This is already used by maiken for windows releases when adding optional github API
  features which require HTTPS

  Example usage is within the [mkn.ram](https://github.com/mkn/mkn.ram) library

  Specifically the "https" profile in the [mkn.yaml](https://github.com/mkn/mkn.ram/blob/master/mkn.yaml) file

**[clang.format](https://github.com/mkn-mod/clang.format)**

  Pre link time hook to run clang-format on your code base to keep it crisp

  Possible addons may include only files currently staged for commit

  See local mkn.yaml for example
