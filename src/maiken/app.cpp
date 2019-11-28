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
#include "maiken/property.hpp"

maiken::Application *maiken::Applications::getOrCreate(const maiken::Project &proj,
                                                       const std::string &_profile, bool setup)
    KTHROW(kul::Exception) {
  std::string pDir(proj.dir().real());
  std::string profile = _profile.empty() ? "@" : _profile;
  if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
    auto app = std::make_unique<Application>(proj, _profile);
    auto pp = app.get();
    m_appPs.push_back(std::move(app));
    m_apps[pDir][profile] = pp;
    if (setup) {
      const std::string &cwd(kul::env::CWD());
      kul::env::CWD(proj.dir());
      pp->setup();
      kul::env::CWD(cwd);
    }
  }
  return m_apps[pDir][profile];
}

maiken::Application *maiken::Applications::getOrCreateRoot(const maiken::Project &proj,
                                                           const std::string &_profile, bool setup)
    KTHROW(kul::Exception) {
  std::string pDir(proj.dir().real());
  std::string profile = _profile.empty() ? "@" : _profile;
  if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
    auto app = std::make_unique<Application>(proj, _profile);
    auto pp = app.get();
    pp->ro = 1;
    m_appPs.push_back(std::move(app));
    m_apps[pDir][profile] = pp;
    if (setup) {
      const std::string &cwd(kul::env::CWD());
      kul::env::CWD(proj.dir());
      pp->setup();
      kul::env::CWD(cwd);
    }
  }
  return m_apps[pDir][profile];
}

maiken::Application *maiken::Applications::getOrNullptr(const std::string &project) {
  uint8_t count = 0;
  Application *app = nullptr;
  for (const auto p1 : m_apps)
    for (const auto p2 : p1.second) {
      if (p2.second->project().root()[STR_NAME].Scalar() == project) {
        count++;
        app = p2.second;
      }
    }
  if (count > 1) {
    KEXIT(1, "Cannot deduce project version as")
        << " there are multiple versions in the dependency tree";
  }
  return app;
}

kul::cli::EnvVar maiken::Application::PARSE_ENV_NODE(YAML::Node const &n, Application *app) {
  using namespace kul::cli;
  KLOG(INF);
  if (n.IsScalar()) {
    auto bits = kul::String::SPLIT(n.Scalar(), "=");
    if (bits.size() != 2) KEXIT(1, "NOOOO");
    auto ev = EnvVar(bits[0], bits[1], EnvVarMode::REPL);
    KLOG(INF) << ev.name();
    return ev;
  } else if (n.IsSequence()) {
    EnvVarMode mode = EnvVarMode::PREP;
    if (n[STR_MODE].Scalar().compare(STR_APPEND) == 0)
      mode = EnvVarMode::APPE;
    else if (n[STR_MODE].Scalar().compare(STR_PREPEND) == 0)
      mode = EnvVarMode::PREP;
    else if (n[STR_MODE].Scalar().compare(STR_REPLACE) == 0)
      mode = EnvVarMode::REPL;
    auto ev = EnvVar(n[STR_NAME].Scalar(),
                     app ? Properties::RESOLVE(*app, n[STR_VALUE].Scalar()) : n[STR_VALUE].Scalar(),
                     mode);
    KLOG(INF) << ev.name();
    return ev;
  }
  KEXIT(1, "NO");
}
