#!/usr/bin/env bash

set -ex
shell_session_update() { :; }
CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD


make bsd CXX="clang++"
CXXFLAGS="-std=c++17 -fPIC"
KLOG=3 ./mkn build -WOdKta "$CXXFLAGS" -g 0
