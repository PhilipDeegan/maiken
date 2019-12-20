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
  static void RUN(const Application &a, std::string bin, const std::string buildDir,
                  compiler::Mode m, bool dbg = 0) {
    kul::File f(bin, buildDir);
    if (!f) KEXIT(1, "binary does not exist \n" + f.full());
    std::unique_ptr<kul::Process> p;
    if (dbg) {
      std::string dbg = kul::env::GET("MKN_DBG");
      if (dbg.empty())
        if (Settings::INSTANCE().root()[STR_LOCAL] &&
            Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER])
          dbg = Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER].Scalar();
      if (dbg.empty()) {
#ifdef _WIN32
        p = std::make_unique<kul::Process>("cdb");
        p->arg("-o");
#else
        p = std::make_unique<kul::Process>("gdb");
#endif
      } else {
        std::vector<std::string> bits(kul::cli::asArgs(dbg));
        p = std::make_unique<kul::Process>(bits[0]);
        for (uint16_t i = 1; i < bits.size(); i++) p->arg(bits[i]);
      }
      p->arg(f.mini());
    } else
      p = std::make_unique<kul::Process>(f.escm());

    if (AppVars::INSTANCE().runArgs().size()) {
      for (const auto &s : kul::cli::asArgs(AppVars::INSTANCE().runArgs())) p->arg(s);
    } else {
      const auto &cmds = AppVars::INSTANCE().commands();
      if (!cmds.count(STR_BUILD) && !cmds.count(STR_COMPILE)) {
        for (const auto &s : kul::cli::asArgs(AppVars::INSTANCE().args())) p->arg(s);
      }
    }

    std::vector<std::pair<std::string, std::string> > envies;
    if (m != compiler::Mode::STAT) {
      std::string arg;
      for (const auto &s : a.libraryPaths()) arg += s + kul::env::SEP();
      if (!arg.empty()) arg.pop_back();
#if defined(__APPLE__)
      kul::cli::EnvVar dy("DYLD_LIBRARY_PATH", arg, kul::cli::EnvVarMode::PREP);
      KOUT(DBG) << dy.name() << " : " << dy.toString();
      envies.push_back(std::make_pair(dy.name(), dy.toString()));
#endif
#ifdef _WIN32
      kul::cli::EnvVar pa("PATH", arg, kul::cli::EnvVarMode::PREP);
#else
      kul::cli::EnvVar pa("LD_LIBRARY_PATH", arg, kul::cli::EnvVarMode::PREP);
#endif
      KOUT(DBG) << pa.name() << " : " << pa.toString();
      envies.push_back(std::make_pair(pa.name(), pa.toString()));
    }
    for (const auto &ev : envies) {
      p->var(ev.first,
             kul::cli::EnvVar(ev.first, ev.second, kul::cli::EnvVarMode::PREP).toString());
    }
    for (const auto &ev : AppVars::INSTANCE().envVars()) {
      auto it = std::find_if(envies.begin(), envies.end(),
                             [&ev](const std::pair<std::string, std::string> &element) {
                               return element.first == ev.first;
                             });
      if (it == envies.end())
        p->var(ev.first,
               kul::cli::EnvVar(ev.first, ev.second, kul::cli::EnvVarMode::PREP).toString());
    }
    KOUT(DBG) << (*p);
    if (!AppVars::INSTANCE().dryRun()) p->start();
  }
};
}  // namespace maiken

void maiken::Application::test() {
  kul::os::PushDir pushd(this->project().dir());
  kul::Dir testsD(buildDir().join("test"));
  if (testsD) {
    for (const auto &file : testsD.files()) {
      if (file) {
#ifdef _WIN32
        if (file.name().rfind(".exe") == std::string::npos) continue;
#endif
        Runner::RUN(*this, file.name(), testsD.real(), m);
      } else
        Runner::RUN(*this, kul::File(file.full(), testsD).real(), buildDir().real(), m);
    }
  }
}

void maiken::Application::run(bool dbg) {
  if(main.empty()) return;
  std::string bin;
  for (const auto &file : buildDir().files(false)) {
    bin = file.name();
#if defined(_WIN32)
    if (bin.rfind(".exe") == std::string::npos) continue;
#endif
    Runner::RUN(*this, bin, buildDir().real(), m, dbg);
  }
}
