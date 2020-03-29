#!/usr/bin/env bash

set -ex

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

export MKN_GCC_PREFERRED=1
make nix CXX="$CXX"
./mkn build -dta "-std=c++17 -fPIC" -l "-pthread -ldl" -WOg 0
./mkn build -dta "-std=c++17 -fPIC" -l "-pthread -ldl" -WOg 0 -p lib
