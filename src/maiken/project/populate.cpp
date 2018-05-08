

#include "maiken.hpp"


kul::hash::map::S2S maiken::Project::populate_tests(const YAML::Node &node){
  kul::hash::map::S2S tests;

  for(const auto line : kul::String::LINES(node.Scalar())){
    for(const auto file : kul::String::SPLIT(line, ' ')){
      tests.insert(file, file);
    }
  }
  return tests;
}
