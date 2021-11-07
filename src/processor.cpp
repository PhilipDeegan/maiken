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

#include <mutex>

struct ProcInfo {
  mkn::kul::hash::set::String objects;
  maiken::ThreadingCompiler tc;

  ProcInfo(maiken::Application& app) : tc(app) {}
};

void maiken::Processor::process(std::vector<Application*> apps) {
  KLOG(DBG) << "BETA PROCESSOR";

  auto const& cmds = CommandStateMachine::INSTANCE().commands();

  std::unordered_map<Application*, std::shared_ptr<ProcInfo>> app_info;

  auto o = [](std::string const& s) {
    if (s.size()) KOUT(NON) << s;
  };
  auto e = [](std::string const& s) {
    if (s.size()) KERR << s;
  };

  std::mutex mute;
  std::vector<CompilerProcessCapture> cpcs;
  mkn::kul::ChroncurrentThreadPool<> ctp(AppVars::INSTANCE().threads(), 1, 1000000000, 1000);
  std::vector<maiken::CompilationUnit> c_units;

  auto lambex = [&](mkn::kul::Exception const&) {
    ctp.stop();
    ctp.interrupt();
  };
  auto lambda = [o, e, &mute, &lambex, &cpcs](maiken::CompilationUnit const& c_unit) {
    CompilerProcessCapture const cpc = c_unit.compile();
    if (!AppVars::INSTANCE().dryRun()) {
      if (mkn::kul::LogMan::INSTANCE().inf() || cpc.exception()) o(cpc.outs());
      if (mkn::kul::LogMan::INSTANCE().inf() || cpc.exception()) e(cpc.errs());
      KOUT(INF) << cpc.cmd();
    } else
      KOUT(NON) << cpc.cmd();
    std::lock_guard<std::mutex> lock(mute);
    cpcs.push_back(cpc);
    try {
      if (cpc.exception()) std::rethrow_exception(cpc.exception());
    } catch (mkn::kul::Exception const& e) {
      lambex(e);
    } catch (const std::exception& e) {
      KLOG(ERR) << e.what();
    }
  };

  if (cmds.count(STR_BUILD) || cmds.count(STR_COMPILE))
    for (auto* apP : apps) {
      auto& app = *apP;
      mkn::kul::os::PushDir pushd(app.project().dir());

      for (auto& modLoader : app.mods)
        modLoader->module()->compile(app, app.modCompile(modLoader->app()));

      app_info.emplace(apP, std::make_shared<ProcInfo>(app));

      if (cmds.count(STR_CLEAN) && app.buildDir().is()) {
        mkn::kul::Dir(app.buildDir().join(".mkn")).rm();
        app.buildDir().rm();
      }
      app.loadTimeStamps();

      SourceFinder s_finder(app);
      auto sources = app.sourceMap();
      CompilerValidation::check_compiler_for(app, sources);
      std::vector<mkn::kul::File> cacheFiles;
      auto& objects = app_info.at(apP)->objects;
      for (auto const& pair : s_finder.all_sources_from(sources, objects, cacheFiles)) {
        auto unit = app_info[apP]->tc.compilationUnit(pair);
        mkn::kul::this_thread::nSleep(5000000);  // dup appears to be overloaded with too many threads
        ctp.async(std::bind(lambda, unit), std::bind(lambex, std::placeholders::_1));
      }
    }

  ctp.finish(1000000 * 1000);
  if (ctp.exception()) KEXIT(1, "Compile error detected");

  if (cmds.count(STR_BUILD) || cmds.count(STR_LINK))
    for (auto* apP : apps) {
      auto& objects = app_info.at(apP)->objects;
      auto& app = *apP;
      for (auto& modLoader : app.mods)
        modLoader->module()->link(app, app.modLink(modLoader->app()));
      app.findObjects(objects);
      app.link(objects);
    }
}
