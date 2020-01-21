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

maiken::Application::Application(const maiken::Project &proj, const std::string &profile)
    : m(compiler::Mode::NONE), p(profile), proj(proj) {
  ps["MKN_ROOT"] = proj.dir().real();
}

maiken::Application::~Application() {
  for (auto mod : mods) mod->unload();
}

void maiken::Application::resolveLang() KTHROW(maiken::Exception) {
  const auto &mains(inactiveMains());
  if (mains.size())
    lang = (*mains.begin()).substr((*mains.begin()).rfind(".") + 1);
  else if (sources().size()) {
    const auto srcMM = sourceMap();
    std::string maxS;
    kul::hash::map::S2T<size_t> mapS;
    size_t maxI = 0, maxO = 0;
    for (const auto &ft : srcMM) mapS.insert(ft.first, 0);
    for (const auto &ft : srcMM) mapS[ft.first] = mapS[ft.first] + ft.second.size();
    for (const auto &s_i : mapS)
      if (s_i.second > maxI) {
        maxI = s_i.second;
        maxS = s_i.first;
      }
    for (const auto s_i : mapS)
      if (s_i.second == maxI) maxO++;
    if (maxO > 1)
      KEXCEPSTREAM << "file type conflict: linker filetype cannot be deduced, "
                   << "specify lang tag to override\n"
                   << project().dir().path();
    lang = maxS;
  }
}

kul::hash::set::String maiken::Application::inactiveMains() const {
  kul::hash::set::String iMs;
  std::string p;
  try {
    p = kul::Dir::REAL(main);
  } catch (const kul::Exception &e) {
  }
  std::string f;
  try {
    if (project().root()[STR_MAIN]) {
      f = kul::Dir::REAL(project().root()[STR_MAIN].Scalar());
      if (p.compare(f) != 0) iMs.insert(f);
    }
  } catch (const kul::Exception &e) {
  }
  for (const YAML::Node &c : project().root()[STR_PROFILE]) {
    try {
      if (c[STR_MAIN]) {
        f = kul::Dir::REAL(c[STR_MAIN].Scalar());
        if (p.compare(f) != 0) iMs.insert(f);
      }
    } catch (const kul::Exception &e) {
    }
  }
  return iMs;
}

void maiken::Application::populateMaps(const YAML::Node &n)
    KTHROW(kul::Exception) {  // IS EITHER ROOT OR PROFILE NODE!

  {
    using namespace kul::cli;
    if (n[STR_ENV]) {
      if (n[STR_ENV].IsScalar())
        evs.emplace_back(PARSE_ENV_NODE(n[STR_ENV], this));
      else
        for (const auto &c : n[STR_ENV]) evs.emplace_back(PARSE_ENV_NODE(c, this));
    }

    for (const auto &p : AppVars::INSTANCE().envVars()) {
      if (std::find_if(evs.begin(), evs.end(),
                       [&p](const EnvVar &ev) { return ev.name() == p.first; }) != evs.end())
        continue;
      evs.push_back(EnvVar(p.first, p.second, EnvVarMode::REPL));
    }
  }

  if (n[STR_ARG])
    for (const auto &o : kul::String::LINES(n[STR_ARG].Scalar()))
      arg += Properties::RESOLVE(*this, o) + " ";
  if (n[STR_LINK])
    for (const auto &o : kul::String::LINES(n[STR_LINK].Scalar()))
      lnk += Properties::RESOLVE(*this, o) + " ";
  try {
    if (n[STR_INC])
      for (const auto &o : kul::String::LINES(n[STR_INC].Scalar())) addIncludeLine(o);
  } catch (const kul::StringException &) {
    KEXIT(1, "include contains invalid bool value\n" + project().dir().path());
  }
  try {
    if (n[STR_SRC])
      for (const auto &o : kul::String::LINES(n[STR_SRC].Scalar())) addSourceLine(o);
  } catch (const kul::StringException &) {
    KEXIT(1, "source contains invalid bool value\n" + project().dir().path());
  }
  if (n[STR_PATH])
    for (const auto &s : kul::String::SPLIT(n[STR_PATH].Scalar(), ' '))
      if (s.size()) {
        kul::Dir d(Properties::RESOLVE(*this, s));
        if (d)
          paths.push_back(d.escr());
        else
          KEXIT(1, "library path does not exist\n" + d.path() + "\n" + project().dir().path());
      }

  if (n[STR_LIB])
    for (const auto &s : kul::String::SPLIT(n[STR_LIB].Scalar(), ' '))
      if (s.size())
        for (const auto &S : kul::String::SPLIT(Properties::RESOLVE(*this, s), ' '))
          if (S.size()) libs.push_back(S);

  for (const std::string &s : libraryPaths())
    if (!kul::Dir(s).is()) KEXIT(1, s + " is not a valid directory\n" + project().dir().path());
}

void maiken::Application::cyclicCheck(
    const std::vector<std::pair<std::string, std::string>> &apps) const KTHROW(kul::Exception) {
  if (par) par->cyclicCheck(apps);
  for (const auto &pa : apps) {
    if (project().dir() == pa.first && p == pa.second)
      KEXIT(1, "Cyclical dependency found\n" + project().dir().path());
  }
}

void maiken::Application::addIncludeLine(const std::string &o) KTHROW(kul::Exception) {
  auto add_if_missing = [&](std::pair<std::string, bool> p) {
    auto it = std::find_if(
        incs.begin(), incs.end(),
        [&](const std::pair<std::string, bool> &element) { return element.first == p.first; });
    if (it == incs.end()) incs.emplace_back(p);
  };
  if (o.find(',') == std::string::npos) {
    for (const auto &s : kul::cli::asArgs(o))
      if (s.size()) {
        auto str(Properties::RESOLVE(*this, s));
        kul::Dir d(str);
        kul::File f(str);
        if (d)
          add_if_missing(std::make_pair(d.real(), true));
        else if (f)
          add_if_missing(std::make_pair(f.real(), false));
        else {
          KEXIT(1, "include does not exist\n" + str + "\n" + project().dir().path());
        }
      }
  } else {
    std::vector<std::string> v;
    kul::String::SPLIT(o, ",", v);
    if (v.size() == 0 || v.size() > 2)
      KEXIT(1, "include invalid format\n" + project().dir().path());
    auto str(Properties::RESOLVE(*this, v[0]));
    kul::Dir d(str);
    kul::File f(str);
    if (d)
      add_if_missing(std::make_pair(d.real(), kul::String::BOOL(v[1])));
    else if (f)
      KEXIT(1,
            "include file does not support CSV syntax\n\t" + str + "\n" + project().dir().path());
    else
      KEXIT(1, "include does not exist\n" + d.path() + "\n" + project().dir().path());
  }
}

void maiken::Application::setSuper() {
  if (sup) return;
  if (project().root()[STR_SUPER]) {
    kul::os::PushDir pushd(project().dir().real());
    kul::Dir d(project().root()[STR_SUPER].Scalar());
    if (!d) KEXIT(1, "Super does not exist in project: " + project().dir().real());
    std::string super(d.real());
    if (super == project().dir().real())
      KEXIT(1, "Super cannot reference itself: " + project().dir().real());
    d = kul::Dir(super);
    try {
      sup = Applications::INSTANCE().getOrCreate(*maiken::Projects::INSTANCE().getOrCreate(d), "");
      sup->resolveProperties();
    } catch (const std::exception &e) {
      KEXIT(1, "Possible super cycle detected: " + project().dir().real());
    }
    auto cycle = sup;
    while (cycle) {
      if (cycle->project().dir() == project().dir())
        KEXIT(1, "Super cycle detected: " + project().dir().real());
      cycle = cycle->sup;
    }
    for (const auto &p : sup->properties())
      if (!ps.count(p.first)) ps.insert(p.first, p.second);
  }
  for (const auto &p : Settings::INSTANCE().properties())
    if (!ps.count(p.first)) ps.insert(p.first, p.second);
}
