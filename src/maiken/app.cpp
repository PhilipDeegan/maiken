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
#include "maiken/env.hpp"

maiken::Application* maiken::Applications::getOrCreate(maiken::Project const& proj,
                                                       std::string const& _profile, bool setup)
    KTHROW(mkn::kul::Exception) {
  std::string pDir(proj.dir().real());
  std::string profile = _profile.empty() ? "@" : _profile;
  if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
    auto app = std::make_unique<Application>(proj, _profile);
    auto pp = app.get();
    m_appPs.push_back(std::move(app));
    m_apps[pDir][profile] = pp;
    if (setup) {
      mkn::kul::os::PushDir pushd(proj.dir());
      pp->setup();
    }
  }
  return m_apps[pDir][profile];
}

maiken::Application* maiken::Applications::getOrCreateRoot(maiken::Project const& proj,
                                                           std::string const& _profile, bool setup)
    KTHROW(mkn::kul::Exception) {
  std::string pDir(proj.dir().real());
  std::string profile = _profile.empty() ? "@" : _profile;
  if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
    auto* pp = getOrCreate(proj, _profile, /*setup = */ false);
    pp->ro = 1;
    if (setup) {
      mkn::kul::os::PushDir pushd(proj.dir());
      pp->setup();
    }
  }
  return m_apps[pDir][profile];
}

maiken::Application* maiken::Applications::getOrNullptr(std::string const& project) {
  uint32_t count = 0;
  Application* app = nullptr;
  for (auto const& p1 : m_apps)
    for (auto const& p2 : p1.second) {
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

mkn::kul::cli::EnvVar maiken::Application::PARSE_ENV_NODE(YAML::Node const& n, Application const& app) {
  return maiken::PARSE_ENV_NODE(n, app, app.project().file());
}



