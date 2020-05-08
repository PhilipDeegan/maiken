#!/usr/bin/env bash

set -ex

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

export MKN_GCC_PREFERRED=1
make nix CXX="$CXX"
CXXFLAGS="-std=c++17 -fPIC"
KLOG=3 ./mkn build -WOdKta "$CXXFLAGS" -g 0 -p static
