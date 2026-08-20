/**
Copyright (c) 2026, Philip Deegan.
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
#include "maiken.hpp"

std::vector<mkn::kul::env::Var> maiken::Application::requiredEnv() const {
  std::vector<mkn::kul::env::Var> vars;
  if (mode() == compiler::Mode::STAT) return vars;
  if (!Settings::INSTANCE().propertyBool("mkn.env.automatic")) return vars;

  std::string arg;
  for (auto const& s : libraryPaths()) arg += s + mkn::kul::env::SEP();
  if (!arg.empty()) arg.pop_back();

#if defined(__APPLE__)
  vars.emplace_back("DYLD_LIBRARY_PATH", arg, mkn::kul::env::Var::Mode::PREP);
#endif
#ifdef _WIN32
  vars.emplace_back("PATH", arg, mkn::kul::env::Var::Mode::PREP);
#else
  vars.emplace_back("LD_LIBRARY_PATH", arg, mkn::kul::env::Var::Mode::PREP);
#endif
  return vars;
}
