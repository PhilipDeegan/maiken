#!/usr/bin/env bash

set -ex
shell_session_update() { :; }
CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

make bsd CXX="clang++"
./mkn build -dta "-std=c++14 -fPIC" -l "-pthread -ldl" -WOg 0 merge
./mkn build -dta "-std=c++14 -fPIC" -l "-pthread -ldl" -WOg 0 -p lib
