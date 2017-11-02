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

void
maiken::Application::addCLIArgs(const kul::cli::Args& args)
{
  auto addIncsOrPaths = [&args](Application& a) {

    auto splitPathAndCheck = [](const std::string& path,
                                const std::string& type) {
      const auto v(kul::String::SPLIT(path, kul::env::SEP()));
      for (const auto s : v) {
        kul::Dir d(s);
        if (!d)
          KEXIT(1, type + " directory does not exist: ") << s;
      }
      return v;
    };

    if (args.has(STR_FINC))
      for (const auto& s :
           splitPathAndCheck(args.get(STR_FINC), "front include"))
        a.incs.insert(a.incs.begin(), std::make_pair(s, true));
    if (args.has(STR_BINC))
      for (const auto& s :
           splitPathAndCheck(args.get(STR_BINC), "back include"))
        a.incs.push_back(std::make_pair(s, true));
    if (args.has(STR_FPATH))
      for (const auto& s : splitPathAndCheck(args.get(STR_FPATH), "front path"))
        a.paths.insert(a.paths.begin(), s);
    if (args.has(STR_BPATH))
      for (const auto& s : splitPathAndCheck(args.get(STR_BPATH), "back path"))
        a.paths.push_back(s);
  };

  addIncsOrPaths(*this);
  for (auto* d : deps)
    addIncsOrPaths(*d);
}


void
maiken::Application::withArgs(
  std::vector<YAML::Node>& with_nodes, 
  std::function<void(const YAML::Node& n, const bool mod)> getIfMissing
)
{

  if (this->ro && AppVars::INSTANCE().with().size()) {
    kul::hash::set::String withs;
    try {
      parseDependencyString(AppVars::INSTANCE().with(), withs);
    } catch (const kul::Exception& e) {
      auto with(AppVars::INSTANCE().with());
      if(with[0] == '[' && with[with.size() - 1] == ']')
        withs.insert(this->project().root()[STR_NAME].Scalar() + with);
      else 
        KEXIT(1, MKN_ERR_INVALID_WIT_CLI);
    }
    for (const auto& with : withs) {
      YAML::Node node;
      std::string local, profiles, proj = with, version, scm;
      {
        auto lbrak = proj.find("(");
        auto rbrak = proj.find(")");
        if (lbrak != std::string::npos) {
          if(rbrak == std::string::npos) KEXIT(1, "Invalid -w - missing right bracket");
          scm = proj.substr(lbrak + 1, rbrak - lbrak - 1);
          proj = proj.substr(0, lbrak) + with.substr(rbrak + 1);
          node[STR_SCM] = scm;
        }
        lbrak = proj.find("[");
        rbrak = proj.find("]");
        if (lbrak != std::string::npos) {
          if(rbrak == std::string::npos) KEXIT(1, "Invalid -w - missing right bracket");
          profiles = proj.substr(lbrak + 1, rbrak - lbrak - 1);
          proj = proj.substr(0, lbrak);
          kul::String::REPLACE_ALL(profiles, ",", " ");
          node[STR_PROFILE] = profiles;
        }
      }
      auto am = proj.find("&"); // local
      auto ha = proj.find("#"); // version
      if (proj == this->project().root()[STR_NAME].Scalar()) {
        node[STR_LOCAL] = ".";
        if(am != std::string::npos || ha != std::string::npos) 
          KEXIT(1, "-w invalid, current project may not specify version or location");
      }

      if(am != std::string::npos && ha != std::string::npos) 
        if(ha > am) KEXIT(1, "-w invalid, version must before location");
      
      if(am != std::string::npos){
        local = proj.substr(am + 1);
        proj = proj.substr(0, am);
        node[STR_LOCAL] = local;
      }
      if(ha != std::string::npos){
        version = proj.substr(ha + 1);
        proj = proj.substr(0, ha);
        node[STR_VERSION] = version;
      }
      if(proj.empty() && local.empty() && scm.empty()) 
        KEXIT(1, "-w invalid, project cannot be deduced");
      if(!proj.empty()){
        if(scm.empty()){
          scm = proj;
          node[STR_SCM] = scm;
        }
        auto bits(kul::String::SPLIT(proj, "/"));
        proj = bits[bits.size() - 1];
      }
      else
      if(proj.empty() && !scm.empty()){
        auto bits(kul::String::SPLIT(scm, "/"));
        proj = bits[bits.size() - 1];
      }
      if(!proj.empty()) node[STR_NAME] = proj;

      with_nodes.push_back(node);
      if (!proj.empty()) {
        std::stringstream with_define;
        kul::String::REPLACE_ALL(proj, ".", "_");
        std::transform(
          proj.begin(), proj.end(), proj.begin(), ::toupper);
        with_define << " -D_MKN_WITH_" << proj << "_ ";
        arg += with_define.str();
      }
      getIfMissing(node, 0);
    }
  }
}
