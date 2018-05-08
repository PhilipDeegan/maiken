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
// #include "kul/log.hpp"

#include "maiken.hpp"

class Validator : public maiken::Constants {
 private:
  std::vector<std::string> ifArgsLefts{"bin", "lib", "shared", "static"};
  std::vector<std::string> ifOSLefts{"bsd", "nix", "win"};
  Validator() {
    std::vector<std::string> ifLefts;
    for (const auto& s : maiken::Compilers::INSTANCE().keys())
      ifArgsLefts.push_back(s);
    for (const auto& s : ifOSLefts)
      for (const std::string& a : ifArgsLefts) ifLefts.push_back(s + "_" + a);
    for (const auto& s : ifLefts) ifArgsLefts.push_back(s);
    for (const auto& s : ifOSLefts) ifArgsLefts.push_back(s);
  }
  static Validator& INSTANCE() {
    static Validator v;
    return v;
  }
  static void IF_VALUEDATER(const maiken::Application& a, const YAML::Node& n,
                            const std::string& s,
                            const std::vector<std::string>& lefts) {
    kul::hash::set::String keys;
    for (YAML::const_iterator it = n.begin(); it != n.end(); ++it) {
      if (std::find(lefts.begin(), lefts.end(), it->first.Scalar()) ==
          lefts.end())
        KEXIT(1, "malformed " + s + " key, \n" + a.project().dir().path());
      if (keys.count(it->first.Scalar()))
        KEXIT(1, "Duplicate " + s + "key detected: " + it->first.Scalar() +
                     "\n" + a.project().dir().path());
      keys.insert(it->first.Scalar());
    }
  }

 public:
  static void PRE_BUILD(const maiken::Application& a, const YAML::Node& n)
      KTHROW(maiken::Exception) {
    // if (n[STR_MAIN] && n[STR_LANG])
    //   KEXIT(1,
    //         "cannot have both main and lang tag\n" + a.project().dir().path());
    // if (n[STR_MAIN]) {
    //   const std::string& m(n[STR_MAIN].Scalar());
    //   if (m.find(".") == std::string::npos)
    //     KEXIT(1, "main tag invalid format, expects <file>.<type>\n" +
    //                  a.project().dir().path());
    // }
    if (n[STR_MODE]) {
      const auto& s(n[STR_MODE].Scalar());
      if (s != STR_NONE && s != STR_STATIC && s != STR_SHARED)
        KEXIT(1, "mode tag invalid value, expects none/static/shared\n" +
                     a.project().dir().path());
    }
    if (n[STR_IF_ARG])
      IF_VALUEDATER(a, n[STR_IF_ARG], STR_IF_ARG, INSTANCE().ifArgsLefts);
    if (n[STR_IF_LNK])
      IF_VALUEDATER(a, n[STR_IF_LNK], STR_IF_LNK, INSTANCE().ifArgsLefts);
    if (n[STR_IF_INC])
      IF_VALUEDATER(a, n[STR_IF_INC], STR_IF_INC, INSTANCE().ifOSLefts);
    if (n[STR_IF_SRC])
      IF_VALUEDATER(a, n[STR_IF_SRC], STR_IF_SRC, INSTANCE().ifOSLefts);
    if (n[STR_IF_LIB])
      IF_VALUEDATER(a, n[STR_IF_LIB], STR_IF_LIB, INSTANCE().ifOSLefts);
    if (n[STR_IF_DEP])
      IF_VALUEDATER(a, n[STR_IF_DEP], STR_IF_DEP, INSTANCE().ifOSLefts);
    if (n[STR_IF_MOD])
      IF_VALUEDATER(a, n[STR_IF_MOD], STR_IF_MOD, INSTANCE().ifOSLefts);
    if (n[STR_DEP])
      for (const auto& d : n[STR_DEP])
        if (!d[STR_LOCAL] && !d[STR_NAME])
          KEXIT(1, "dependency name must exist if local tag does not\n" +
                       a.project().dir().path());
  }
  static void POST_BUILD(const maiken::Application& a, const YAML::Node& n)
      KTHROW(maiken::Exception) {
    std::stringstream ss;
    for (const auto f : a.files()) ss << f.first << " ";
    if (n[STR_MAIN] &&
        !a.files().count(
            n[STR_MAIN].Scalar().substr(n[STR_MAIN].Scalar().rfind(".") + 1)))
      KEXIT(1, "main tag invalid type, valid types are\n" + ss.str() + "\n" +
                   a.project().dir().path());
    else if (n[STR_LANG] && !a.files().count(n[STR_LANG].Scalar()))
      KEXIT(1, "lang tag invalid type, valid types are\n" + ss.str() + "\n" +
                   a.project().dir().path());

  }
  static bool PARENT_CYCLE(const maiken::Application& a, const std::string& pr,
                           const std::string& pa) {
    for (const auto& p1 : a.project().root()[STR_PROFILE]) {
      if (p1[STR_NAME].Scalar() != pr) continue;
      if (p1[STR_PARENT]) {
        std::string resolved(
            maiken::Properties::RESOLVE(a, p1[STR_PARENT].Scalar()));
        if (resolved == pa)
          return true;
        else
          return PARENT_CYCLE(a, resolved, pa);
      }
    }
    return false;
  }
  static void SELF_CHECK(const maiken::Application& a, const YAML::Node& n,
                         const std::vector<std::string>& profiles) {
    if (n[STR_SELF])
      for (const auto& s : kul::String::SPLIT(
               maiken::Properties::RESOLVE(a, n[STR_SELF].Scalar()), ' '))
        if (std::find(profiles.begin(), profiles.end(), s) == profiles.end())
          KEXIT(1, "Self tag references unknown profile: \"" + s + "\" in " +
                       a.project().dir().path());
  }
};

void maiken::Application::preSetupValidation() KTHROW(maiken::Exception) {
  {
    kul::hash::set::String keys;
    for (YAML::const_iterator it = project().root()[STR_PROPERTY].begin();
         it != project().root()[STR_PROPERTY].end(); ++it) {
      if (keys.count(it->first.Scalar()))
        KEXIT(1, "Duplicate PROPERTIES: " + it->first.Scalar() + "\n" +
                     project().dir().path());
      keys.insert(it->first.Scalar());
    }
  }
  bool dpp = project().root()[STR_PARENT];
  bool dpf = 0;
  Validator::PRE_BUILD(*this, project().root());
  std::vector<std::string> profiles;
  for (const auto& profile : project().root()[STR_PROFILE]) {
    const std::string& p(profile[STR_NAME].Scalar());
    if (p.find("[") != std::string::npos || p.find("]") != std::string::npos)
      KEXIT(1, "Profile may not contain character \"[\" or \"]\"");
    if (p == project().root()[STR_NAME].Scalar())
      KEXIT(1, "Profile may not have same name as project");
    if (std::find(profiles.begin(), profiles.end(), p) != profiles.end())
      KEXIT(1, "Duplicate profile name found");
    profiles.push_back(p);
    if (profile[STR_PARENT]) {
      bool f = 0;
      std::string resolved(
          Properties::RESOLVE(*this, profile[STR_PARENT].Scalar()));
      if (resolved == profile[STR_NAME].Scalar())
        KEXIT(1, "Profile may not be its own parent");
      for (const auto& p1 : project().root()[STR_PROFILE]) {
        if (profile[STR_NAME].Scalar() == p1[STR_NAME].Scalar()) continue;
        if (resolved == p1[STR_NAME].Scalar()) f = 1;
        if (f) {
          if (Validator::PARENT_CYCLE(*this, p1[STR_NAME].Scalar(),
                                      profile[STR_NAME].Scalar()))
            KEXIT(1, "Profile inheritence cycle detected");
          break;
        }
      }
      if (!f)
        KEXIT(1, "parent profile not found: " +
                     Properties::RESOLVE(*this, profile[STR_PARENT].Scalar()) +
                     "\n" + project().dir().path());
    }
    Validator::PRE_BUILD(*this, profile);
    if (dpp && !dpf)
      dpf = Properties::RESOLVE(*this, project().root()[STR_PARENT].Scalar()) ==
            p;
  }
  if (dpp && !dpf)
    KEXIT(1, "Parent for default profile does not exist: \n" +
                 project().dir().path());
  Validator::SELF_CHECK(*this, project().root(), profiles);
  for (const auto& n : project().root()[STR_PROFILE])
    Validator::SELF_CHECK(*this, n, profiles);
}

void maiken::Application::postSetupValidation() KTHROW(maiken::Exception) {
  Validator::POST_BUILD(*this, project().root());
  for (const auto& profile : project().root()[STR_PROFILE])
    Validator::POST_BUILD(*this, profile);
}
