

#include "maiken.hpp"

kul::hash::map::S2S maiken::Project::populate_tests(YAML::Node const& node) {
  kul::hash::map::S2S tests;

  for (auto const& line : kul::String::LINES(node.Scalar())) {
    for (auto const& file : kul::String::SPLIT(line, ' ')) {
      tests.insert(file, file);
    }
  }
  return tests;
}
