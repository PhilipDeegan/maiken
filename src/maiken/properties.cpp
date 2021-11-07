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
#include "maiken.hpp"

void maiken::Application::resolveProperties() KTHROW(maiken::Exception) {
  ps.setDeletedKey("--DELETED--");
  for (YAML::const_iterator it = project().root()[STR_PROPERTY].begin();
       it != project().root()[STR_PROPERTY].end(); ++it)
    ps[it->first.as<std::string>()] = it->second.as<std::string>();
  for (YAML::const_iterator it = project().root()[STR_PROPERTY].begin();
       it != project().root()[STR_PROPERTY].end(); ++it) {
    std::string s = Properties::RESOLVE(*this, it->second.as<std::string>());
    if (ps.count(it->first.as<std::string>())) ps.erase(it->first.as<std::string>());
    ps[it->first.as<std::string>()] = s;
  }
}

std::shared_ptr<std::tuple<std::string, int, int>> maiken::Properties::KEY(
    mkn::kul::hash::map::S2S const& /*ps*/, std::string const& s) KTHROW(mkn::kul::Exception) {
  std::string r = s;
  int lb = s.find("${");
  int clb = s.find("\\${");
  int rb = s.find("}");
  int crb = s.find("\\}");
  while ((lb - clb + 1) == 0) {
    lb = r.substr(clb + 3).find("${");
    clb = r.substr(clb + 3).find("\\${");
  }
  while ((rb - crb + 1) == 0) {
    rb = r.substr(crb + 2).find("}");
    crb = r.substr(crb + 2).find("\\}");
  }
  if (lb != -1 && clb == -1 && rb != -1 && crb == -1)
    return std::make_shared<std::tuple<std::string, int, int>>(r.substr(lb + 2, rb - 2 - lb), lb,
                                                               rb);
  return 0;
}

std::string maiken::Properties::RESOLVE(Application const& app, std::string const& s)
    KTHROW(mkn::kul::Exception) {
  std::string r = s;
  std::shared_ptr<std::tuple<std::string, int, int>> t = KEY(app.properties(), s);
  if (t) {
    std::string k = std::get<0>(*t);
    int const& lb = std::get<1>(*t);
    int const& rb = std::get<2>(*t);

    if (AppVars::INSTANCE().properkeys().count(k))
      k = AppVars::INSTANCE().properkeys().at(k);
    else if (app.properties().count(k) == 0) {
      if (app.project().root()[k] && app.project().root()[k].Type() == 2)
        k = app.project().root()[k].Scalar();
      else
        KEXIT(1, "Property : '" + k + "' has not been defined");
    } else
      k = app.properties().at(k);
    r = Properties::RESOLVE(app, r.substr(0, lb) + k + r.substr(rb + 1));
  }
  mkn::kul::String::TRIM(r);
  return r;
}

std::string maiken::Properties::RESOLVE(Settings const& set, std::string const& s)
    KTHROW(mkn::kul::Exception) {
  std::string r = s;
  std::shared_ptr<std::tuple<std::string, int, int>> t = KEY(set.properties(), s);
  if (t) {
    std::string k = std::get<0>(*t);
    int const& lb = std::get<1>(*t);
    int const& rb = std::get<2>(*t);

    if (AppVars::INSTANCE().properkeys().count(k))
      k = AppVars::INSTANCE().properkeys().at(k);
    else if (set.properties().count(k) > 0)
      k = set.properties().at(k);
    else
      KEXIT(1, "Property : '" + k + "' has not been defined");
    r = Properties::RESOLVE(set, r.substr(0, lb) + k + r.substr(rb + 1));
  }
  mkn::kul::String::TRIM(r);
  return r;
}
