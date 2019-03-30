#!/usr/bin/env bash

set -ex

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

export MKN_GCC_PREFERRED=1
sudo ln -s /usr/bin/gcc-7 /usr/local/bin/gcc; sudo ln -s /usr/bin/g++-7 /usr/local/bin/g++; gcc -v;
make nix CXX=/usr/bin/g++-7
