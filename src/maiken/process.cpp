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

#ifdef _MKN_WITH_MKN_RAM_
#include "maiken/github.hpp"
bool maiken::Application::get_binaries() {
  size_t suxcess = 0;
  kul::Dir outD(inst ? inst.real() : buildDir());
  const auto files = kul::String::SPLIT(this->binary(), " ");
  for (const std::string &file : files) {
    std::string fn = file.substr(file.rfind("/") + 1);
    kul::https::Get(file)
        .withHeaders({{"User-Agent", "Mozilla not a virus"},
                      {"Accept", "application/octet-stream"},
                      {"Content-Disposition", "attachment; filename=" + fn}})
        .withResponse([&](const kul::http::Response &r) {
          if (r.status() == 200) {
            kul::File dl(fn);
            if (dl.is()) {
              suxcess++;
              dl.mv(outD);
            }
          }
        })
        .send();
  }
  return suxcess == files.size();
}
#endif

namespace maiken {
class ObjectMerger {
 public:
  static void into(const Application & root) {
    kul::Dir robj(root.buildDir().join("obj"));
    for (auto a : root.dependencies()){
      kul::Dir obj(a->buildDir().join("obj"));
      if(obj) for(auto f : obj.files()) f.cp(robj);
    }
  }
};
}

void maiken::Application::process() KTHROW(kul::Exception) {
  const kul::hash::set::String &cmds(CommandStateMachine::INSTANCE().commands());
  const auto gEnvVars = maiken::AppVars::INSTANCE().envVars();
  kul::os::PushDir pushd(this->project().dir());
  auto loadModules = [&](Application &app) {
#ifndef _MKN_DISABLE_MODULES_
    for (auto mod = app.modDeps.begin(); mod != app.modDeps.end(); ++mod) {
      app.mods.push_back(ModuleLoader::LOAD(**mod));
    }
    for (auto &modLoader : app.mods) modLoader->module()->init(app, app.modInit(modLoader->app()));
#endif  //_MKN_DISABLE_MODULES_
  };
  auto proc = [&](Application &app, bool work) {
    kul::env::CWD(app.project().dir());

    if (work) {
      if (!app.buildDir()) app.buildDir().mk();
      if (BuildRecorder::INSTANCE().has(app.buildDir().real())) return;
      BuildRecorder::INSTANCE().add(app.buildDir().real());
    }
    kul::Dir mkn(app.buildDir().join(".mkn"));
    std::vector<std::pair<std::string, std::string>> oldEvs;
    for (const auto &ev : app.envVars()) {
      const std::string v = kul::env::GET(ev.name());
      oldEvs.push_back(std::pair<std::string, std::string>(ev.name(), v));
      kul::env::SET(ev.name(), ev.toString().c_str());
      maiken::AppVars::INSTANCE().envVar(ev.name(), ev.toString());
    }
    if (cmds.count(STR_CLEAN) && app.buildDir().is()) {
      app.buildDir().rm();
      mkn.rm();
    }
    if (cmds.count(STR_MERGE) && app.ro) ObjectMerger::into(app);
#ifdef _MKN_WITH_MKN_RAM_
    if (work && !app.bin.empty() && app.get_binaries()) work = false;  // doesn't work yet
#endif
    app.loadTimeStamps();

    kul::hash::set::String objects;
    if (cmds.count(STR_BUILD) || cmds.count(STR_COMPILE)) {
      for (auto &modLoader : app.mods)
        modLoader->module()->compile(app, app.modCompile(modLoader->app()));
      if (work) app.compile(objects);
    }
    if (cmds.count(STR_BUILD) || cmds.count(STR_LINK)) {
      if (work)
        for (auto &modLoader : app.mods)
          modLoader->module()->link(app, app.modLink(modLoader->app()));
      if (cmds.count(STR_MERGE) && app.ro) {
        app.findObjects(objects);
        app.link(objects);
      }
    }
    for (const auto &oldEv : oldEvs) kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
    for (const auto e : gEnvVars) maiken::AppVars::INSTANCE().envVar(e.first, e.second);
  };

  auto _mods = ModuleMinimiser::modules(*this);
  for (auto &mod : ModuleMinimiser::modules(*this)) {
    bool build = mod.second->is_build_required();
    bool is_build_stale = mod.second->is_build_stale();
    if (!build && (is_build_stale && !maiken::AppVars::INSTANCE().quiet())) {
      std::stringstream ss;
      ss << "The project @ " << mod.second->project().dir() << " appears to be stale" << std::endl;
      ss << "\tWould you like to build it (Y/n) - this message can be removed "
            "with -q"
         << std::endl;
      build = kul::String::BOOL(kul::cli::receive(ss.str()));
    }
    if (build) {
      CommandStateMachine::INSTANCE().main(0);
      for (auto &m : _mods) m.second->process();
      CommandStateMachine::INSTANCE().main(1);
    }
  }
  for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app) loadModules(**app);
  loadModules(*this);
  for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app) {
    if ((*app)->ig) continue;
    if ((*app)->lang.empty()) (*app)->resolveLang();
    (*app)->main.clear();
    proc(**app, !(*app)->srcs.empty());
  }
  if (!this->ig) proc(*this, (!this->srcs.empty() || !this->main.empty()));
  if (cmds.count(STR_TEST)) test();
  if (cmds.count(STR_PACK)) {
    pack();
    for (auto &modLoader : mods) modLoader->module()->pack(*this, this->modPack(modLoader->app()));
  }
  if (CommandStateMachine::INSTANCE().main() && (cmds.count(STR_RUN) || cmds.count(STR_DBG)))
    run(cmds.count(STR_DBG));
  CommandStateMachine::INSTANCE().reset();
}
