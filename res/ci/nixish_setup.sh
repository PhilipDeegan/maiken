#!/usr/bin/env bash
set -exu
CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" && cd "$CWD"/../..
MKN_KUL_GIT=${MKN_KUL_GIT:-master}
YAML_CPP_GIT=${YAML_CPP_GIT:-master}

rm -rf bin && mkdir bin
[ -d "./ext/mkn/kul" ] || \
    git clone --depth 1 https://github.com/mkn/mkn.kul.git -b "${MKN_KUL_GIT}" "ext/mkn/kul";

[ -d "ext/parse/yaml" ] || (
  git clone --depth 1 https://github.com/mkn/parse.yaml.git --branch "${YAML_CPP_GIT}" "ext/parse/yaml";
  cd "ext/parse/yaml" && ./mkn.sh && mkdir -p p/bin
)
