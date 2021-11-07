/**
Copyright (c) 2017, Philip Deegan.
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
                                  std::function<void(YAML::Node const&, const bool)> getIfMissing) {
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
                              std::function<void(YAML::Node const&, const bool)> getIfMissing) {
  for (auto mod : mods) {
    mkn::kul::String::REPLACE_ALL(mod, mkn::kul::os::EOL(), "");
    mkn::kul::String::TRIM(mod);
    if (mod.empty()) continue;

    mod_nodes.emplace_back();
    auto& node = mod_nodes.back();
    std::string local /*&*/, profiles, proj = mod, version /*#*/, scm, objs;

    auto get_between = [&](auto& var, auto lbrak, auto rbrak) {
      auto between = maiken::string::between_rm_str(proj, lbrak, rbrak);
      if (between.found) proj = between.remaining, var = *between.found;
      return !between.error;
    };

    if (!get_between(scm, "(", ")")) KEXIT(1, "Invalid -m - missing right ) bracket");
    if (!node[STR_SCM]) node[STR_SCM] = scm;

    if (!get_between(profiles, "[", "]")) KEXIT(1, "Invalid -m - missing right ] bracket");
    mkn::kul::String::REPLACE_ALL(profiles, ",", " ");
    if (!node[STR_PROFILE] && profiles.size()) node[STR_PROFILE] = profiles;

    {
      auto lbrak = proj.find("{"), rbrak = proj.rfind("}");
      if (lbrak != std::string::npos) {
        if (rbrak == std::string::npos) KEXIT(1, "Invalid -m - missing right } bracket");
        objs = proj.substr(lbrak), proj = proj.substr(0, lbrak);
      }

      auto am = proj.find("&"), ha = proj.find("#");
      if (proj == this->project().root()[STR_NAME].Scalar()) {
        node[STR_LOCAL] = ".";
        if (am != std::string::npos || ha != std::string::npos)
          KEXIT(1,
                "-m invalid, current project may not specify version or "
                "location");
      }

      if (am != std::string::npos && ha != std::string::npos)
        if (ha > am) KEXIT(1, "-m invalid, version must before location");

      auto if_set = [&](auto s, auto& v, auto n) {
        if (s != std::string::npos) v = proj.substr(s + 1), proj = proj.substr(0, s), n = v;
      };
      if_set(am, local, node[STR_LOCAL]);
      if_set(ha, version, node[STR_VERSION]);
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

    if (objs.size())
      for (auto const n : mkn::kul::bon::from(objs))
        for (auto const p : n) node[p.first] = p.second;

    YAML::Emitter out;
    out << node;
    getIfMissing(node, 1);
  }
}
