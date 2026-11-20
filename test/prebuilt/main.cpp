#include "yaml-cpp/yaml.h"

#include <iostream>

int main() {
  auto const n = YAML::Load("a: [1, 2, 3]");
  std::cout << "parsed " << n["a"].size() << std::endl;
  return n["a"].size() == 3 ? 0 : 1;
}
