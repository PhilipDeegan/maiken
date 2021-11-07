

#include "maiken.hpp"

mkn::kul::hash::map::S2S maiken::Project::populate_tests(YAML::Node const& node) {
  mkn::kul::hash::map::S2S tests;

  for (auto const& line : mkn::kul::String::LINES(node.Scalar())) {
    for (auto const& file : mkn::kul::String::SPLIT(line, ' ')) {
      tests.insert(file, file);
    }
  }
  return tests;
}
