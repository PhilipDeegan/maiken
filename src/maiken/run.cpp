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

void
maiken::Application::run(bool dbg)
{
  std::string bin(out.empty() ? project().root()[STR_NAME].Scalar() : out);
#ifdef _WIN32
  bin += ".exe";
#endif
  kul::File f(bin, inst ? inst : buildDir());
  if (!f)
    KEXIT(1, "binary does not exist \n" + f.full());
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
      for (uint16_t i = 1; i < bits.size(); i++)
        p->arg(bits[i]);
    }
    p->arg(f.mini());
  } else
    p = std::make_unique<kul::Process>(f.escm());

  if (AppVars::INSTANCE().runArgs().size()) {
    for (const auto& s : kul::cli::asArgs(AppVars::INSTANCE().runArgs()))
      p->arg(s);
  } else {
    const auto& cmds = AppVars::INSTANCE().commands();
    if (!cmds.count(STR_BUILD) && !cmds.count(STR_BUILD_MOD) &&
        !cmds.count(STR_BUILD_ALL) && !cmds.count(STR_COMPILE)) {
      for (const auto& s : kul::cli::asArgs(AppVars::INSTANCE().args()))
        p->arg(s);
    }
  }

  if (m != compiler::Mode::STAT) {
    std::string arg;
    for (const auto& s : libraryPaths())
      arg += s + kul::env::SEP();
    arg.pop_back();
#ifdef _WIN32
    kul::cli::EnvVar pa("PATH", arg, kul::cli::EnvVarMode::PREP);
#else
    kul::cli::EnvVar pa("LD_LIBRARY_PATH", arg, kul::cli::EnvVarMode::PREP);
#endif
    KOUT(INF) << pa.name() << " : " << pa.toString();
    p->var(pa.name(), pa.toString());
  }
  for (const auto& ev : AppVars::INSTANCE().envVars())
    p->var(ev.first,
           kul::cli::EnvVar(ev.first, ev.second, kul::cli::EnvVarMode::PREP)
             .toString());
  KOUT(INF) << (*p);
  if (!AppVars::INSTANCE().dryRun())
    p->start();
  KEXIT(0, "");
}