// prints the env: variables the settings files of the chain set, see res/ci/build.py
#include <cstdlib>
#include <iostream>

int main() {
  auto const get = [](char const* name) {
    char const* v = std::getenv(name);
    return v ? v : "";
  };
  std::cout << "SS_WHO=" << get("SS_WHO") << " SS_B=" << get("SS_B") << std::endl;
  return 0;
}
