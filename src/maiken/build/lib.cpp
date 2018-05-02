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

maiken::CompilerProcessCapture maiken::Application::buildLibrary(
    const kul::hash::set::String& objects) {
  if (fs.count(lang) > 0) {
    if (m == compiler::Mode::NONE) m = compiler::Mode::SHAR;
    if (!(*files().find(lang)).second.count(STR_COMPILER))
      KEXIT(1, "No compiler found for filetype " + lang);
    std::string linker = fs[lang][STR_LINKER];
    std::string linkEnd;
    if (ro) linkEnd = AppVars::INSTANCE().linker();
    if (!AppVars::INSTANCE().allinker().empty())
      linkEnd += " " + AppVars::INSTANCE().allinker();
    if (!lnk.empty()) linkEnd += " " + lnk;
    if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() &&
        linkEnd.size())
      KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
    if (m == compiler::Mode::STAT) linker = fs[lang][STR_ARCHIVER];
    kul::Dir outD(inst ? inst.real() : buildDir());
    std::string lib(baseLibFilename());
    lib = AppVars::INSTANCE().dryRun() ? kul::File(lib, outD).esc()
                                       : kul::File(lib, outD).escm();
    std::vector<std::string> obV;
    for (const auto& o : objects) obV.emplace_back(o);
    const std::string& base(Compilers::INSTANCE().base(
        (*(*files().find(lang)).second.find(STR_COMPILER)).second));
    if (cLnk.count(base)) linkEnd += " " + cLnk[base];
    auto* comp = Compilers::INSTANCE().get(base);
    auto linkOpt(comp->linkerOptimizationLib(AppVars::INSTANCE().optimise()));
    if (!linkOpt.empty()) linker += " " + linkOpt;
    auto linkDbg(comp->linkerDebugLib(AppVars::INSTANCE().debug()));
    if (!linkDbg.empty()) linker += " " + linkDbg;
    const CompilerProcessCapture& cpc =
        comp->buildLibrary(linker, linkEnd, obV, libraries(), libraryPaths(),
                           lib, m, AppVars::INSTANCE().dryRun());
    if (AppVars::INSTANCE().dryRun())
      KOUT(NON) << cpc.cmd();
    else {
      checkErrors(cpc);
      KOUT(INF) << cpc.cmd();
      KOUT(NON) << "Creating lib: " << kul::File(cpc.file()).real();
    }
    return cpc;
  } else
    KEXCEPTION("Unable to handle artifact: \"" + lang +
               "\" - type is not in file list");
}
