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
#include "maiken/app.hpp"

void maiken::Application::withArgs(
    std::string const with_str, std::vector<YAML::Node>& with_nodes,
    std::function<void(YAML::Node const& n, bool const mod)> getIfMissing, bool dep) {
  if (with_str.size()) {
    mkn::kul::hash::set::String withs;
    try {
      parseDependencyString(with_str, withs);
    } catch (mkn::kul::Exception const& e) {
      if (with_str[0] == '[' && with_str[with_str.size() - 1] == ']')
        withs.insert(this->project().root()[STR_NAME].Scalar() + with_str);
      else
        KEXIT(1, MKN_ERR_INVALID_WIT_CLI);
    }
    with(withs, with_nodes, getIfMissing, dep);
  }
}

void maiken::Application::with(
    mkn::kul::hash::set::String& withs, std::vector<YAML::Node>& with_nodes,
    std::function<void(YAML::Node const& n, bool const mod)> getIfMissing, bool dep) {
  for (auto const& with : withs) {
    YAML::Node node;
    std::string local /*&*/, profiles, proj = with, version /*#*/, scm;

    auto get_between = [&](auto& var, auto lbrak, auto rbrak) {
      auto between = maiken::string::between_rm_str(proj, lbrak, rbrak);
      if (between.found) proj = between.remaining, var = *between.found;
      return !between.error;
    };

    if (!get_between(scm, "(", ")")) KEXIT(1, "Invalid -w - missing right ) bracket");
    if (!node[STR_SCM]) node[STR_SCM] = scm;

    if (!get_between(profiles, "[", "]")) KEXIT(1, "Invalid -w - missing right ] bracket");
    mkn::kul::String::REPLACE_ALL(profiles, ",", " ");
    if (!node[STR_PROFILE]) node[STR_PROFILE] = profiles;

    auto am = proj.find("&"), ha = proj.find("#");
    if (proj == this->project().root()[STR_NAME].Scalar()) {
      node[STR_LOCAL] = ".";
      if (am != std::string::npos || ha != std::string::npos)
        KEXIT(1,
              "-w invalid, current project may not specify version or "
              "location");
    }

    if (am != std::string::npos && ha != std::string::npos)
      if (ha > am) KEXIT(1, "-w invalid, version must before location");

    auto if_set = [&](auto s, auto& v, auto n) {
      if (s != std::string::npos) v = proj.substr(s + 1), proj = proj.substr(0, s), n = v;
    };
    if_set(am, local, node[STR_LOCAL]);
    if_set(ha, version, node[STR_VERSION]);

    if (proj.empty() && local.empty() && scm.empty())
      KEXIT(1, "-w invalid, project cannot be deduced");
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

    with_nodes.push_back(node);
    if (!dep && !proj.empty()) {
      std::stringstream with_define;
      mkn::kul::String::REPLACE_ALL(proj, ".", "_");
      std::transform(proj.begin(), proj.end(), proj.begin(), ::toupper);
      with_define << " -D_MKN_WITH_" << proj << "_ ";
      arg += with_define.str();
    }
    getIfMissing(node, 0);
  }
}
