#!/usr/bin/env bash

set -ex

shell_session_update() { :; }

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

brew update
brew install lcov

make bsd CXX="clang++" CXXFLAGS="--coverage -O0 -g -std=c++14 -fmessage-length=0 -fPIC" LDFLAGS="--coverage"

./mkn build -dta "-std=c++14 -fPIC" -l "-pthread -ldl" -WOg 0 merge
ls -l
llvm-cov gcov -f -b test.gcda
rm test.gcda
./mkn build -dta "-std=c++14 -fPIC" -l "-pthread -ldl" -WOg 0 -p lib
ls -l
llvm-cov gcov -f -b test.gcda
