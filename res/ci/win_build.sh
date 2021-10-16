
set -ex
WHICH=which
[ -z "$INCLUDE" ] && echo "Windows vcvars.bat has not been called" && exit 1
[ -z "$(where git)" ] && echo "no git" && exit 1

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $CWD/../..
ROOT=$PWD

EXE=mkn
KUL_GIT=master
YAML_GIT=pr

YAML_INC="ext/parse/yaml/$YAML_GIT/p/include"
INC=""
INCS=(inc
  $YAML_INC
  ext/mkn/kul/$KUL_GIT/inc
  ext/mkn/kul/$KUL_GIT/os/win/inc
  ext/mkn/kul/$KUL_GIT/os/nixish/inc
)

for i in ${INCS[@]}; do INC+=" -I$i"; done
compile(){ cl -std:c++17 -nologo -EHsc $INC -c -Fo$1 "$2" || exit 1 ; }
archive(){ lib -nologo -LTCG -OUT:"$1" "$2/*.o"; }
exe(){     link -OUT:"mkn.exe" -nologo bin/*.o parse.yaml.lib  \
           -nodefaultlib:libucrt.lib ucrt.lib; rm parse.yaml.lib; }

[ ! -d "ext/mkn/kul/$KUL_GIT" ] && \
  git clone --depth 1 https://github.com/mkn/mkn.kul -b $KUL_GIT ext/mkn/kul/$KUL_GIT

[ ! -d "ext/parse/yaml/$YAML_GIT" ] && \
  git clone --depth 1 https://github.com/mkn/parse.yaml -b $YAML_GIT ext/parse/yaml/$YAML_GIT && \
  cd ./ext/parse/yaml/$YAML_GIT && ./mkn.sh && cd $ROOT

if [ ! -d "ext/parse/yaml/$YAML_GIT/p/bin" ]; then
  mkdir -p ext/parse/yaml/$YAML_GIT/p/bin;
  for f in $(find ext/parse/yaml/$YAML_GIT/p/src -type f -name '*.cpp'); do
    compile "ext/parse/yaml/$YAML_GIT/p/bin/$(basename $f).o" "$f"
  done
  archive "parse.yaml.lib" "ext/parse/yaml/$YAML_GIT/p/bin"
fi

rm -rf bin && mkdir -p bin

for f in $(find src -type f -name '*.cpp'); do compile "bin/$(basename $f).o" "$f"; done
compile "bin/mkn.cpp.o" "mkn.cpp"
exe

rm -rf bin ext
