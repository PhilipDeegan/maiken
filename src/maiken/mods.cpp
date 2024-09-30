/**
Copyright (c) 2022, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, mod or modout
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided mod the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software modout specific prior written permission.

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
#include "mkn/kul/bon.hpp"

void maiken::Application::modArgs(std::string const mod_str, std::vector<YAML::Node>& mod_nodes,
                                  std::function<void(YAML::Node const&, bool const)> getIfMissing) {
  if (mod_str.size()) {
    mkn::kul::hash::set::String mods;
    std::stringstream ss;
    size_t lb = 0, rb = 0;
    for (auto& c : mod_str) {
      rb = c == '}' ? rb + 1 : rb;
      lb = c == '{' ? lb + 1 : lb;
      if ((c == ',' || c == ' ') && rb == lb) {
        mods.insert(ss.str());
        ss.str(std::string());
        lb = rb = 0;
        continue;
      }
      ss << c;
    }
    if (rb != lb) KEXIT(1, "Invalid -m - inconsistent {} brackets");
    if (ss.str().size()) mods.insert(ss.str());
    mod(mods, mod_nodes, getIfMissing);
  }
}

void maiken::Application::mod(mkn::kul::hash::set::String& mods, std::vector<YAML::Node>& mod_nodes,
                              std::function<void(YAML::Node const&, bool const)> getIfMissing) {
  for (auto mod : mods) {
    mkn::kul::String::REPLACE_ALL(mod, mkn::kul::os::EOL(), "");
    mkn::kul::String::TRIM(mod);
    if (mod.empty()) continue;

    auto& node = mod_nodes.emplace_back();

    ProjectInfo pInfo = ProjectInfo::PARSE_LINE(mod);
    auto& [local, profiles, proj, version, scm, curlies] = pInfo;

    if (!node[STR_SCM] && scm.size()) node[STR_SCM] = scm;
    if (!node[STR_PROFILE] && profiles.size()) node[STR_PROFILE] = profiles;

    {
      auto const am = proj.find("&"), ha = proj.find("#");
      if (proj == this->project().root()[STR_NAME].Scalar()) {
        node[STR_LOCAL] = ".";
        if (am != std::string::npos || ha != std::string::npos)
          KEXIT(1,
                "-m invalid, current project may not specify version or "
                "location");
      }

      auto const if_set = [&](auto& v, auto n) {
        if (v.size()) n = v;
      };
      if_set(local, node[STR_LOCAL]);
      if_set(version, node[STR_VERSION]);
    }

    if (proj.empty() && local.empty() && scm.empty())
      KEXIT(1, "-m invalid, project cannot be deduced");
    if (!proj.empty()) {
      if (scm.empty()) {
        scm = proj;
        node[STR_SCM] = scm;
      }
      proj = mkn::kul::String::SPLIT(proj, "/").back();
    } else if (proj.empty() && !scm.empty()) {
      proj = mkn::kul::String::SPLIT(scm, "/").back();
    }
    if (!proj.empty()) node[STR_NAME] = proj;

    if (curlies.size())
      for (auto const p : mkn::kul::bon::from("{" + curlies + "}")) node[p.first] = p.second;

    YAML::Emitter out;
    out << node;
    getIfMissing(node, 1);
  }
}
