#include "dep.hpp"

#include "yaml-cpp/yaml.h"

extern "C" MKN_KUL_PUBLISH int test_mod_dep_value() { return 1234; }

extern "C" MKN_KUL_PUBLISH bool test_mod_dep_yaml_ok() {
  try {
    YAML::Node{}["missing"].as<std::string>();
    return false;
  } catch (YAML::Exception const&) {
    return true;
  }
}
