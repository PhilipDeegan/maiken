#pragma once

#include "mkn/kul/defs.hpp"

extern "C" MKN_KUL_PUBLISH int test_mod_dep_value();

// Exercises a libparse.yaml symbol (YAML::Exception's vtable/RTTI) from this
// dependency's own shared library - unlike the module itself, which only
// ever sees YAML::Node via headers, this .so actually links parse.yaml
// (shared), so loading it needs that lib resolvable too.
extern "C" MKN_KUL_PUBLISH bool test_mod_dep_yaml_ok();
