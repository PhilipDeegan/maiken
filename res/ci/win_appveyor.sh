#!/usr/bin/env bash

set -ex

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

export MKN_CL_PREFERRED=1
export PYTHON=/c/Python36-x64/python.exe
mkn clean build -dtKOg 0 -w mkn.ram[https]
mkn clean build -dtSOg 0 -p lib
$PYTHON /c/Python36-x64/Scripts/flawfinder .
rm -rf bin/build/tmp bin/build/obj
rm -rf bin/lib/tmp bin/lib/obj
mkdir -p win/master
cp bin/build/* win/master
cp bin/lib/* win/master
tar cf mkn.tar win
