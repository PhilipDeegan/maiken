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
#include "kul/bon.hpp"

void maiken::Application::modArgs(
    const std::string mod_str, std::vector<YAML::Node> &mod_nodes,
    std::function<void(const YAML::Node &, const bool)> getIfMissing) {
  if (mod_str.size()) {
    kul::hash::set::String mods;
    mods.insert(mod_str);
    mod(mods, mod_nodes, getIfMissing);
  }
}

void maiken::Application::mod(
    kul::hash::set::String &mods, std::vector<YAML::Node> &mod_nodes,
    std::function<void(const YAML::Node &n, const bool mod)> getIfMissing) {
  for (auto &mod1 : mods) {
    auto mod(mod1);
    kul::String::REPLACE_ALL(mod, kul::os::EOL(), "");
    kul::String::TRIM(mod);
    if (mod.empty()) continue;
    // auto eol = kul::os::EOL();
    // auto loe =std::string(eol.rbegin(), eol.rend());

    // if(mod.rfind(kul::os::EOL()) == mod.size() - eol.size())

    // for(const auto e : loe) if(mod.back() == e) mod.pop_back();
    KLOG(INF) << mod;

    mod_nodes.emplace_back();
    auto &node = mod_nodes.back();
    std::string local, profiles, proj = mod, version, scm, objs;
    auto lbrak = proj.find("("), rbrak = proj.find(")");
    {
      if (lbrak != std::string::npos) {
        if (rbrak == std::string::npos) KEXIT(1, "Invalid -m - missing right ) bracket");
        scm = proj.substr(lbrak + 1, rbrak - lbrak - 1);
        proj = proj.substr(0, lbrak) + mod.substr(rbrak + 1);
        node[STR_SCM] = scm;
      }
      lbrak = proj.find("["), rbrak = proj.find("]");
      if (lbrak != std::string::npos) {
        if (rbrak == std::string::npos) KEXIT(1, "Invalid -m - missing right ] bracket");
        profiles = proj.substr(lbrak + 1, rbrak - lbrak - 1);
        proj = proj.substr(0, lbrak);
        kul::String::REPLACE_ALL(profiles, ",", " ");
        node[STR_PROFILE] = profiles;
      }
      lbrak = proj.find("{"), rbrak = proj.rfind("}");
      if (lbrak != std::string::npos) {
        if (rbrak == std::string::npos) KEXIT(1, "Invalid -m - missing right } bracket");
        objs = proj.substr(lbrak /*, rbrak - lbrak + 1*/);
        proj = proj.substr(0, lbrak);
      }
    }
    auto am = proj.find("&");  // local
    auto ha = proj.find("#");  // version
    if (proj == this->project().root()[STR_NAME].Scalar()) {
      node[STR_LOCAL] = ".";
      if (am != std::string::npos || ha != std::string::npos)
        KEXIT(1,
              "-m invalid, current project may not specify version or "
              "location");
    }

    if (am != std::string::npos && ha != std::string::npos)
      if (ha > am) KEXIT(1, "-m invalid, version must before location");

    if (am != std::string::npos) {
      local = proj.substr(am + 1);
      proj = proj.substr(0, am);
      node[STR_LOCAL] = local;
    }
    if (ha != std::string::npos) {
      version = proj.substr(ha + 1);
      proj = proj.substr(0, ha);
      node[STR_VERSION] = version;
    }
    if (proj.empty() && local.empty() && scm.empty())
      KEXIT(1, "-m invalid, project cannot be deduced");
    if (!proj.empty()) {
      if (scm.empty()) {
        scm = proj;
        node[STR_SCM] = scm;
      }
      auto bits(kul::String::SPLIT(proj, "/"));
      proj = bits[bits.size() - 1];
    } else if (proj.empty() && !scm.empty()) {
      auto bits(kul::String::SPLIT(scm, "/"));
      proj = bits[bits.size() - 1];
    }
    if (!proj.empty()) node[STR_NAME] = proj;
    if (lbrak != std::string::npos) {
      KLOG(INF);
      for (const auto n : kul::bon::from(objs)) {
        KLOG(INF);
        for (const auto p : n) {
          KLOG(INF);
          node[p.first] = p.second;
        }
      }
    }
    YAML::Emitter out;
    out << node;
    KLOG(INF) << kul::os::EOL() << out.c_str();
    getIfMissing(node, 1);
  }
}
