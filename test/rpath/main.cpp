#include <cstdio>

extern "C" int rpath_value();

int main() {
  std::printf("rpath value %d\n", rpath_value());
  return 0;
}
