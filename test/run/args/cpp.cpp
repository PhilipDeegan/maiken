
#include <iostream>

int main(int argc, char* argv[]) {
  std::cout << "ARGS:";
  for (int i = 1; i < argc; ++i) std::cout << "[" << argv[i] << "]";
  std::cout << std::endl;

  return 0;
}
