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

class ModuleMinimiser
{
  friend class maiken::Application;

private:
  void add(const std::vector<maiken::Application*>& mods,
           kul::hash::map::S2T<maiken::Application*>& apps)
  {
    for (auto* const m : mods)
      if (!apps.count(m->buildDir().real()))
        apps.insert(m->buildDir().real(), m);
  }

public:
  static ModuleMinimiser& INSTANCE()
  {
    static ModuleMinimiser a;
    return a;
  }
  kul::hash::map::S2T<maiken::Application*> modules(maiken::Application& app)
  {
    kul::hash::map::S2T<maiken::Application*> apps;
    add(app.moduleDependencies(), apps);
    for (auto dep = app.dependencies().rbegin();
         dep != app.dependencies().rend();
         ++dep)
      add((*dep)->moduleDependencies(), apps);
    return apps;
  }
};

class CommandStateMachine
{
  friend class maiken::Application;

private:
  bool _main = 1;
  kul::hash::set::String cmds;
  CommandStateMachine() { reset(); }
  static CommandStateMachine& INSTANCE()
  {
    static CommandStateMachine a;
    return a;
  }
  void reset()
  {
    cmds.clear();
    for (const auto& s : maiken::AppVars::INSTANCE().commands())
      cmds.insert(s);
  }
  void add(const std::string& s) { cmds.insert(s); }
  const kul::hash::set::String& commands() { return cmds; }
  void main(bool m) { _main = m; }
  bool main() { return _main; }
};

class BuildRecorder
{
  friend class maiken::Application;

private:
  kul::hash::set::String builds;
  static BuildRecorder& INSTANCE()
  {
    static BuildRecorder a;
    return a;
  }
  void add(const std::string& k) { builds.insert(k); }
  bool has(const std::string& k) { return builds.count(k); }
};

void
maiken::Application::process() KTHROW(kul::Exception)
{
  const kul::hash::set::String& cmds(
    CommandStateMachine::INSTANCE().commands());
  const kul::hash::set::String& phase(AppVars::INSTANCE().modulePhases());

  auto loadModules = [&](Application& app) {
#ifndef _MKN_DISABLE_MODULES_
    for (auto mod = app.modDeps.begin(); mod != app.modDeps.end(); ++mod) {
      app.mods.push_back(ModuleLoader::LOAD(**mod));
    }
    for (auto& modLoader : app.mods)
      modLoader->module()->init(app, modLoader->app()->modIArg);
#endif //_MKN_DISABLE_MODULES_
  };
  auto proc = [&](Application& app, bool work) {
    kul::env::CWD(app.project().dir());

    if (work) {
      if (!app.buildDir())
        app.buildDir().mk();
      if (BuildRecorder::INSTANCE().has(app.buildDir().real()))
        return;
      BuildRecorder::INSTANCE().add(app.buildDir().real());
    }

    kul::Dir mkn(app.buildDir().join(".mkn"));
    std::vector<std::pair<std::string, std::string>> oldEvs;
    for (const kul::cli::EnvVar& ev : app.envVars()) {
      const std::string v = kul::env::GET(ev.name());
      oldEvs.push_back(std::pair<std::string, std::string>(ev.name(), v));
      kul::env::SET(ev.name(), ev.toString().c_str());
    }
    if (cmds.count(STR_CLEAN) && app.buildDir().is()) {
      app.buildDir().rm();
      mkn.rm();
    }
    app.loadTimeStamps();
    if (cmds.count(STR_TRIM))
      app.trim();

    kul::hash::set::String objects;
    if (cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD) ||
        cmds.count(STR_COMPILE)) {
      if (phase.count(STR_COMPILE))
        for (auto& modLoader : app.mods)
          modLoader->module()->compile(app, modLoader->app()->modCArg);
      if (work)
        app.compile(objects);
    }
    if (cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD) ||
        cmds.count(STR_LINK)) {
      if (phase.count(STR_LINK))
        for (auto& modLoader : app.mods)
          modLoader->module()->link(app, modLoader->app()->modLArg);
      if (work) {
        app.findObjects(objects);
        app.link(objects);
      }
    }
    for (const std::pair<std::string, std::string>& oldEv : oldEvs)
      kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
  };

  if (cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD_MOD)) {
    auto _mods = ModuleMinimiser::INSTANCE().modules(*this);
    if (_mods.size() && !cmds.count(STR_BUILD_ALL))
      CommandStateMachine::INSTANCE().add(STR_BUILD);
    CommandStateMachine::INSTANCE().main(0);
    for (auto& m : _mods)
      m.second->process();
    CommandStateMachine::INSTANCE().main(1);
  }

  for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app)
    loadModules(**app);  
  loadModules(*this);

  for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app) {
    if ((*app)->ig)
      continue;
    if ((*app)->lang.empty())
      (*app)->resolveLang();
    (*app)->main.clear();
    proc(**app, !(*app)->srcs.empty());
  }
  if (!this->ig)
    proc(*this, (!this->srcs.empty() || !this->main.empty()));

  if (cmds.count(STR_PACK)) {
    pack();
    if (phase.count(STR_PACK))
      for (auto& modLoader : mods)
        modLoader->module()->pack(*this, modLoader->app()->modPArg);
  }
  if (CommandStateMachine::INSTANCE().main() &&
      (cmds.count(STR_RUN) || cmds.count(STR_DBG)))
    run(cmds.count(STR_DBG));
  CommandStateMachine::INSTANCE().reset();
}
