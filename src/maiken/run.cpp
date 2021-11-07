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

namespace maiken {
class Runner : public Constants {
 public:
  static void RUN(Application const& a, std::string bin, std::string const buildDir,
                  compiler::Mode m, bool dbg = 0) {
    mkn::kul::File f(bin, buildDir);
    if (!f) KEXIT(1, "binary does not exist \n" + f.full());
    std::unique_ptr<mkn::kul::Process> p;
    if (dbg) {
      std::string dbg_bin = mkn::kul::env::GET("MKN_DBG");
      if (dbg_bin.empty())
        if (Settings::INSTANCE().root()[STR_LOCAL] &&
            Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER])
          dbg_bin = Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER].Scalar();
      if (dbg_bin.empty()) {
#ifdef _WIN32
        p = std::make_unique<mkn::kul::Process>("cdb");
        p->arg("-o");
#else
        p = std::make_unique<mkn::kul::Process>("gdb");
        (*p) << "--args";
#endif
      } else {
        std::vector<std::string> bits(mkn::kul::cli::asArgs(dbg_bin));
        p = std::make_unique<mkn::kul::Process>(bits[0]);
        for (size_t i = 1; i < bits.size(); i++) p->arg(bits[i]);
      }
      p->arg(f.mini());
    } else
      p = std::make_unique<mkn::kul::Process>(f.escm());

    if (AppVars::INSTANCE().runArgs().size()) {
      for (auto const& s : mkn::kul::cli::asArgs(AppVars::INSTANCE().runArgs())) p->arg(s);
    } else {
      auto const& cmds = AppVars::INSTANCE().commands();
      if (!cmds.count(STR_BUILD) && !cmds.count(STR_COMPILE)) {
        for (auto const& s : mkn::kul::cli::asArgs(AppVars::INSTANCE().args())) p->arg(s);
      }
    }

    std::vector<std::pair<std::string, std::string> > envies;
    if (m != compiler::Mode::STAT) {
      std::string arg;
      for (auto const& s : a.libraryPaths()) arg += s + mkn::kul::env::SEP();
      if (!arg.empty()) arg.pop_back();
#if _MKN_DISABLE_RUN_LIB_PATH_HANDLING_ == 0
#if defined(__APPLE__)
      mkn::kul::cli::EnvVar dy("DYLD_LIBRARY_PATH", arg, mkn::kul::cli::EnvVarMode::PREP);
      KOUT(DBG) << dy.name() << " : " << dy.toString();
      envies.push_back(std::make_pair(dy.name(), dy.toString()));
#endif
#ifdef _WIN32
      mkn::kul::cli::EnvVar pa("PATH", arg, mkn::kul::cli::EnvVarMode::PREP);
#else
      mkn::kul::cli::EnvVar pa("LD_LIBRARY_PATH", arg, mkn::kul::cli::EnvVarMode::PREP);
#endif
      KOUT(DBG) << pa.name() << " : " << pa.toString();
      envies.push_back(std::make_pair(pa.name(), pa.toString()));
#endif  // _MKN_DISABLE_RUN_LIB_PATH_HANDLING_
    }
    for (auto const& ev : envies) {
      p->var(ev.first,
             mkn::kul::cli::EnvVar(ev.first, ev.second, mkn::kul::cli::EnvVarMode::PREP).toString());
    }
    for (auto const& ev : AppVars::INSTANCE().envVars()) {
      auto it = std::find_if(envies.begin(), envies.end(),
                             [&ev](std::pair<std::string, std::string> const& element) {
                               return element.first == ev.first;
                             });
      if (it == envies.end())
        p->var(ev.first,
               mkn::kul::cli::EnvVar(ev.first, ev.second, mkn::kul::cli::EnvVarMode::PREP).toString());
    }
    KOUT(DBG) << (*p);
    if (!AppVars::INSTANCE().dryRun()) p->set(a.envVars()).start();
  }
};
}  // namespace maiken

void maiken::Application::test() {
  mkn::kul::os::PushDir pushd(this->project().dir());
  mkn::kul::Dir testsD(buildDir().join("test"));
  if (testsD) {
    for (auto const& file : testsD.files()) {
      if (file) {
#ifdef _WIN32
        if (file.name().rfind(".exe") == std::string::npos) continue;
#endif
        Runner::RUN(*this, file.name(), testsD.real(), m);
      } else
        Runner::RUN(*this, mkn::kul::File(file.full(), testsD).real(), buildDir().real(), m);
    }
  }
}

void maiken::Application::run(bool dbg) {
  if (!main_) return;
  std::string bin;
  for (auto const& file : buildDir().files(false)) {
    bin = file.name();
#if defined(_WIN32)
    if (bin.rfind(".exe") == std::string::npos) continue;
#endif
    Runner::RUN(*this, bin, buildDir().real(), m, dbg);
  }
}
