/**
Copyright (c) 2017, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kul/io.hpp"

#include "maiken/defs.hpp"
#include "maiken/project.hpp"

const kul::yaml::Validator
maiken::Project::validator() const
{
  using namespace kul::yaml;

  std::vector<NodeValidator> depVals{ NodeValidator("name"),
                                      NodeValidator("version"),
                                      NodeValidator("profile"),
                                      NodeValidator("scm"),
                                      NodeValidator("local") };
  NodeValidator dep("dep", depVals, 0, NodeType::LIST);
  NodeValidator if_dep("if_dep",
                       { NodeValidator("*", depVals, 0, NodeType::LIST) },
                       0,
                       NodeType::MAP);

  std::vector<NodeValidator> modVals{
    NodeValidator("name"),
    NodeValidator("version"),
    NodeValidator("profile"),
    NodeValidator("scm"),
    NodeValidator("local"),
    NodeValidator(
      "arg",
      { 
        NodeValidator("init", { NodeValidator("*") }, 0, NodeType::NON),
        NodeValidator("compile", { NodeValidator("*") }, 0, NodeType::NON),
        NodeValidator("link", { NodeValidator("*") }, 0, NodeType::NON),
        NodeValidator("pack", { NodeValidator("*") }, 0, NodeType::NON) },
      0,
      NodeType::MAP)
  };
  NodeValidator mod("mod", modVals, 0, NodeType::LIST);
  NodeValidator if_mod("if_mod",
                       { NodeValidator("*", modVals, 0, NodeType::LIST) },
                       0,
                       NodeType::MAP);

  NodeValidator env("env",
                    { NodeValidator("name", 1),
                      NodeValidator("mode", 1),
                      NodeValidator("value", 1) },
                    0,
                    NodeType::LIST);

  NodeValidator if_arg("if_arg", { NodeValidator("*") }, 0, NodeType::MAP);
  NodeValidator if_inc("if_inc", { NodeValidator("*") }, 0, NodeType::MAP);
  NodeValidator if_lib("if_lib", { NodeValidator("*") }, 0, NodeType::MAP);
  NodeValidator if_src("if_src", { NodeValidator("*") }, 0, NodeType::MAP);
  NodeValidator if_lnk("if_link", { NodeValidator("*") }, 0, NodeType::MAP);

  return Validator(
    { NodeValidator("name", 1),
      NodeValidator("version"),
      NodeValidator("scm"),
      NodeValidator("property", { NodeValidator("*") }, 0, NodeType::MAP),
      NodeValidator("super"),
      NodeValidator("parent"),
      NodeValidator("inc"),
      NodeValidator("src"),
      NodeValidator("path"),
      NodeValidator("lib"),
      NodeValidator("link"),
      NodeValidator("lang"),
      NodeValidator("main"),
      NodeValidator("mode"),
      NodeValidator("arg"),
      NodeValidator("install"),
      NodeValidator("out"),
      NodeValidator("ext"),
      NodeValidator("self"),
      env,
      dep,
      if_dep,
      mod,
      if_mod,
      if_arg,
      if_inc,
      if_lib,
      if_src,
      if_lnk,
      NodeValidator("profile",
                    { NodeValidator("name", 1),
                      NodeValidator("parent"),
                      NodeValidator("inc"),
                      NodeValidator("src"),
                      NodeValidator("path"),
                      NodeValidator("lib"),
                      NodeValidator("link"),
                      NodeValidator("lang"),
                      NodeValidator("main"),
                      NodeValidator("mode"),
                      NodeValidator("arg"),
                      NodeValidator("install"),
                      NodeValidator("out"),
                      NodeValidator("ext"),
                      NodeValidator("self"),
                      env,
                      dep,
                      if_dep,
                      mod,
                      if_mod,
                      if_arg,
                      if_inc,
                      if_lib,
                      if_src,
                      if_lnk },
                    0,
                    NodeType::LIST) });
}

void
maiken::NewProject::write()
{
  {
    kul::io::Writer w(file());
    w.write("\n", true);
    w.write("name: new_mkn_project", true);
    w.write("#inc: ./inc\n#src: ./src", true);
    w.write("main: cpp.cpp", true);
  }
  {
    kul::io::Writer w("cpp.cpp");
    w.write("\n\n#include <iostream>", true);
    w.write("\n", true);
    w.write("int main(int argc, char* argv[]){", true);
    w.write("\n", true);
    w.write("    std::cout << \"HELLO WORLD!\" << std::endl;", true);
    w.write("\n", true);
    w.write("    return 0;", true);
    w.write("}", true);
  }
}