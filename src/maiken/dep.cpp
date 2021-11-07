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
#include "maiken/graph.hpp"

void includeDependency(std::string const& s, std::string const& p,
                       mkn::kul::hash::set::String& include) {
  if (s == "+") {
    include.insert(s);
  } else {
    for (auto profile : mkn::kul::String::SPLIT(p, ' ')) {
      std::stringstream ss;
      ss << s << "[" << profile << "]";
      include.insert(ss.str());
    }
  }
}

void maiken::Application::parseDependencyString(std::string s, mkn::kul::hash::set::String& include) {
  mkn::kul::String::REPLACE_ALL(s, " ", "");
  std::stringstream dep, pro;
  bool lB = 0, rB = 0;
  for (auto const& c : s) {
    if (c == '[' && (lB || rB))
      KEXIT(1, MKN_ERR_SQRBRKT_MISMATCH_DEP_CLI);
    else if (c == '[' && !lB) {
      if (dep.str().empty()) KEXIT(1, MKN_ERR_INVALID_DEP_CLI);
      lB = 1;
    } else if (c == ']') {
      if (pro.str().empty()) KEXIT(1, MKN_ERR_INVALID_DEP_CLI);
      lB = 0, rB = 1;
      includeDependency(dep.str(), pro.str(), include);
      pro.str(std::string());
      dep.str(std::string());
    } else if (c == ',') {
      if (lB) {
        includeDependency(dep.str(), pro.str(), include);
        pro.str(std::string());
      } else if (rB) {
        rB = 0;
      } else {
        includeDependency(dep.str(), "@", include);
        dep.str(std::string());
      }
    } else {
      if (!lB)
        dep << c;
      else
        pro << c;
    }
  }
  if (!rB) includeDependency(dep.str(), "@", include);
  if (lB) KEXIT(1, MKN_ERR_SQRBRKT_MISMATCH_DEP_CLI);
}

void maiken::Application::buildDepVec(std::string const& depVal) {
  mkn::kul::hash::set::String all, ignore, include;
  ignore.insert("+");

  if (!depVal.empty()) {
    try {
      AppVars::INSTANCE().dependencyLevel(mkn::kul::String::UINT16(depVal));
    } catch (const mkn::kul::StringException& e) {
      AppVars::INSTANCE().dependencyLevel(0);
      parseDependencyString(depVal, include);
    }
  }

  if (include.size() == 1 && include.count("+")) {
    AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());
    this->ig = 1;
  }

  uint16_t i = 0;
  std::unordered_map<uint16_t, std::vector<Application*>> dePs;

  for (Application* ap : deps) {
    Application& a(*ap);
    a.buildDepVecRec(dePs, AppVars::INSTANCE().dependencyLevel(), i, include);
    std::string const& name(a.project().root()[STR_NAME].Scalar());
    std::stringstream ss;
    ss << name << "[" << (a.p.empty() ? "@" : a.p) << "]";
    if (AppVars::INSTANCE().dependencyLevel() || include.count(ss.str())) a.ig = 0;
    all.insert(ss.str());
  }

  for (auto const& d : include)
    if (!all.count(d) && !ignore.count(d) && !isMod)
      KEXIT(1, "Dependency project specified does not exist: " + d);
  if (include.size() && include.count("+")) this->ig = 1;

  std::function<void(Application*)> dep_libs;

  dep_libs = [&dep_libs](Application* app) {
    app->deps = DepGrapher{}.build(*app);

    for (auto* depP : app->deps) {
      auto const& dep(*depP);

      if (!dep.sources().empty()) {
        auto lib = dep.baseLibFilename();
        auto const& it(std::find(app->libraries().begin(), app->libraries().end(), lib));
        if (it != app->libraries().end()) app->libs.erase(it);
        app->libs.push_back(lib);
      }

      for (std::string const& s : dep.libraries())
        if (std::find(app->libraries().begin(), app->libraries().end(), s) ==
            app->libraries().end())
          app->libs.push_back(s);

      dep_libs(depP);
    }
  };

  dep_libs(this);
}

void maiken::Application::buildDepVecRec(
    std::unordered_map<uint16_t, std::vector<Application*>>& dePs, int16_t ig, int16_t i,
    mkn::kul::hash::set::String const& inc) {
  for (auto* const a : deps) {
    auto name = a->project().root()[STR_NAME].Scalar();
    std::stringstream ss;
    ss << name << "[" << (a->p.empty() ? name : a->p) << "]";
    if (ig > 0 || inc.count(name) || inc.count(ss.str())) a->ig = 0;
    dePs[i].push_back(a);
    a->buildDepVecRec(dePs, --ig, (++i)--, inc);
  }
}

void maiken::Application::populateMapsFromDependencies() KTHROW(mkn::kul::Exception) {
  for (auto depP = dependencies().rbegin(); depP != dependencies().rend(); ++depP) {
    auto const& dep(**depP);

    for (auto const& s : dep.includes())
      if (s.second && std::find(includes().begin(), includes().end(), s) == includes().end())
        incs.emplace_back(s.first, true);

    for (std::string const& s : dep.libraryPaths())
      if (std::find(libraryPaths().begin(), libraryPaths().end(), s) == libraryPaths().end())
        paths.push_back(s);
  }
}
