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
#include "maiken/github.hpp"

void maiken::Application::addCLIArgs(mkn::kul::cli::Args const& args) {
  auto addIncsOrPaths = [&args](Application& a) {
    auto splitPathAndCheck = [](std::string const& path, std::string const& type) {
      auto const v(mkn::kul::String::SPLIT(path, mkn::kul::env::SEP()));
      for (auto const& s : v) {
        mkn::kul::Dir d(s);
        if (!d) KEXIT(1, type + " directory does not exist: ") << s;
      }
      return v;
    };

    if (args.has(STR_FINC))
      for (auto const& s : splitPathAndCheck(args.get(STR_FINC), "front include"))
        a.incs.insert(a.incs.begin(), std::make_pair(s, true));
    if (args.has(STR_BINC))
      for (auto const& s : splitPathAndCheck(args.get(STR_BINC), "back include"))
        a.incs.push_back(std::make_pair(s, true));
    if (args.has(STR_FPATH))
      for (auto const& s : splitPathAndCheck(args.get(STR_FPATH), "front path"))
        a.paths.insert(a.paths.begin(), s);
    if (args.has(STR_BPATH))
      for (auto const& s : splitPathAndCheck(args.get(STR_BPATH), "back path"))
        a.paths.push_back(s);
  };

  addIncsOrPaths(*this);
  for (auto* d : deps) addIncsOrPaths(*d);
}
