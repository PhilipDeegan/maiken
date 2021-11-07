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
#include "maiken/dist.hpp"

#include <mutex>

namespace maiken {

class Executioner : public Constants {
  friend class Application;

  static CompilerProcessCapture build_exe(mkn::kul::hash::set::String const& objects,
                                          std::vector<mkn::kul::Dir> const& starDirs,
                                          std::string const& main, std::string const& out,
                                          const mkn::kul::Dir outD, Application& app) {
    auto dryRun = AppVars::INSTANCE().dryRun();
    auto& file = main;
    std::string const& fileType = file.substr(file.rfind(".") + 1);

    if (!app.files().at(fileType).count(STR_COMPILER))
      KEXIT(1, "No compiler found for filetype " + fileType);
    if (!dryRun && mkn::kul::LogMan::INSTANCE().inf() && !app.libraries().empty()) {
      KOUT(NON) << "LIBRARIES";
      for (std::string const& s : app.libraries()) KOUT(NON) << "\t" << s;
    }
    if (!dryRun && mkn::kul::LogMan::INSTANCE().inf() && !app.libraryPaths().empty()) {
      KOUT(NON) << "LIBRARY PATHS";
      for (std::string const& s : app.libraryPaths()) KOUT(NON) << "\t" << s;
    }
    try {
      std::string linker = app.fs[fileType][STR_LINKER];
      std::string linkEnd;
      if (app.ro) linkEnd = AppVars::INSTANCE().linker();
      if (!AppVars::INSTANCE().allinker().empty()) linkEnd += " " + AppVars::INSTANCE().allinker();
      if (!app.lnk.empty()) linkEnd += " " + app.lnk;
      if (!dryRun && mkn::kul::LogMan::INSTANCE().inf() && linkEnd.size())
        KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
      std::string bin(dryRun ? mkn::kul::File(outD.join(out)).esc() : mkn::kul::File(outD.join(out)).escm());
      std::vector<std::string> obV;
      for (auto const& o : objects) obV.emplace_back(o);
      auto const& base(Compilers::INSTANCE().base(app.files().at(fileType).at(STR_COMPILER)));
      if (app.cLnk.count(base)) linkEnd += " " + app.cLnk[base];
      auto* comp = Compilers::INSTANCE().get(base);
      auto linkOpt(comp->linkerOptimizationBin(AppVars::INSTANCE().optimise()));
      if (!linkOpt.empty()) linker += " " + linkOpt;
      auto linkDbg(comp->linkerDebugBin(AppVars::INSTANCE().debug()));
      if (!linkDbg.empty()) linker += " " + linkDbg;

      LinkDAO dao{app,   linker, linkEnd, bin, starDirs, obV, app.libraries(), app.libraryPaths(),
                  app.m, dryRun};

      return comp->buildExecutable(dao);
    } catch (CompilerNotFoundException const& e) {
      KEXCEPTION("UNSUPPORTED COMPILER EXCEPTION");
    }
  }

  static void print(CompilerProcessCapture const& cpc, Application& app) {
    auto dryRun = AppVars::INSTANCE().dryRun();

    mkn::kul::Dir cmdLogDir(".mkn/log/" + app.buildDir().name() + "/bin/cmd", 1);
    mkn::kul::Dir outLogDir(".mkn/log/" + app.buildDir().name() + "/bin/out", 1);
    mkn::kul::Dir errLogDir(".mkn/log/" + app.buildDir().name() + "/bin/err", 1);

    if (dryRun)
      KOUT(NON) << cpc.cmd();
    else {
      app.checkErrors(cpc);
      KOUT(INF) << cpc.cmd();
      KOUT(NON) << "Creating bin: " << mkn::kul::File(cpc.file()).real();

      if (AppVars::INSTANCE().dump()) {
        std::string base = mkn::kul::File(cpc.file()).name();
        mkn::kul::io::Writer(mkn::kul::File(base + ".txt", cmdLogDir)) << cpc.cmd();
        if (cpc.outs().size()) mkn::kul::io::Writer(mkn::kul::File(base + ".txt", outLogDir)) << cpc.outs();
        if (cpc.errs().size()) mkn::kul::io::Writer(mkn::kul::File(base + ".txt", errLogDir)) << cpc.errs();
      }

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
      if (AppVars::INSTANCE().nodes()) DistLinker::send(cpc.file());
#endif
    }
  }
};
}  // namespace maiken

void maiken::Application::buildExecutable(mkn::kul::hash::set::String const& objects_)
    KTHROW(mkn::kul::Exception) {
  mkn::kul::hash::set::String objects;
  auto& file = main_->in();
  std::string const& fileType = file.substr(file.rfind(".") + 1);
  if (fs.count(fileType) == 0)
    KEXIT(1, "Unable to handle artifact: \"" + file + "\" - type is not in file list");
  std::string const oType("." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
  mkn::kul::Dir objD(buildDir().join("obj")), tmpD(buildDir().join("tmp"));
  std::string const& name(out.empty() ? project().root()[STR_NAME].Scalar() : out);
  auto obFile = Source(mkn::kul::File(main_->in()).real()).object();
  mkn::kul::File tbject(obFile, tmpD);
  if (!tbject)
    KERR << "Source expected not found (ignoring) " << tbject;
  else {
    objects.emplace(tbject.escm());
  }
  auto install = mkn::kul::Dir(inst ? inst.real() : buildDir());

  std::vector<mkn::kul::Dir> starDirs;
  if (objects_.size()) starDirs.emplace_back(objD);

  auto cpc = Executioner::build_exe(objects, starDirs, file, name, install, *this);
  Executioner::print(cpc, *this);
}

void maiken::Application::buildTest(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception) {
  mkn::kul::Dir objD(buildDir().join("obj"));
  mkn::kul::Dir tmpD(buildDir().join("tmp"), 1);
  mkn::kul::Dir testsD(buildDir().join("test"), 1);

  std::vector<mkn::kul::Dir> starDirs;
  if (objects.size()) starDirs.emplace_back(objD);

  std::mutex mute;
  std::vector<CompilerProcessCapture> cpcs;

  auto build_test = [&](auto& to, auto& testsD, auto& tmpD, auto* app) {
    mkn::kul::hash::set::String cobjects;
    cobjects.insert(mkn::kul::File(to.object(), tmpD).escm());
    mkn::kul::File inFile(to.in());
    auto out = Application::hash(inFile.dir().real()) + "_" + inFile.name();
    auto cpc = Executioner::build_exe(cobjects, starDirs, to.in(), out, testsD, *app);
    std::lock_guard<std::mutex> lock(mute);
    cpcs.push_back(cpc);
  };

  mkn::kul::ChroncurrentThreadPool<> ctp(AppVars::INSTANCE().threads(), 1, 1000000, 1000);

  for (auto const& test : SourceFinder(*this).tests())
    ctp.async(std::bind(build_test, test, testsD, tmpD, this));
  ctp.finish(10000000);  // 10 milliseconds

  for (auto const& cpc : cpcs) Executioner::print(cpc, *this);
}

maiken::CompilerProcessCapture maiken::Application::buildLibrary(
    mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception) {
  auto dryRun = AppVars::INSTANCE().dryRun();
  if (fs.count(lang) > 0) {
    mkn::kul::Dir objD(buildDir().join("obj"));
    if (m == compiler::Mode::NONE) m = compiler::Mode::SHAR;
    if (!files().at(lang).count(STR_COMPILER)) KEXIT(1, "No compiler found for filetype " + lang);
    std::string linker = fs[lang][STR_LINKER], linkEnd;
    if (ro) linkEnd = AppVars::INSTANCE().linker();
    if (!AppVars::INSTANCE().allinker().empty()) linkEnd += " " + AppVars::INSTANCE().allinker();
    if (!lnk.empty()) linkEnd += " " + lnk;
    if (!dryRun && mkn::kul::LogMan::INSTANCE().inf() && linkEnd.size())
      KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
    if (m == compiler::Mode::STAT) linker = fs[lang][STR_ARCHIVER];
    mkn::kul::Dir outD(inst ? inst.real() : buildDir());
    std::string lib(baseLibFilename());
    lib = dryRun ? mkn::kul::File(lib, outD).esc() : mkn::kul::File(lib, outD).escm();
    auto const& base(Compilers::INSTANCE().base(files().at(lang).at(STR_COMPILER)));
    if (cLnk.count(base)) linkEnd += " " + cLnk[base];
    auto* comp = Compilers::INSTANCE().get(base);
    auto linkOpt(comp->linkerOptimizationLib(AppVars::INSTANCE().optimise()));
    if (!linkOpt.empty()) linker += " " + linkOpt;
    auto linkDbg(comp->linkerDebugLib(AppVars::INSTANCE().debug()));
    if (!linkDbg.empty()) linker += " " + linkDbg;

    std::vector<mkn::kul::Dir> starDirs;
    if (objects.size()) starDirs.emplace_back(objD);
    std::vector<std::string> obV;
    LinkDAO dao{*this, linker, linkEnd, lib, starDirs, obV, libraries(), libraryPaths(), m, dryRun};

    CompilerProcessCapture const& cpc = comp->buildLibrary(dao);
    if (dryRun)
      KOUT(NON) << cpc.cmd();
    else {
      checkErrors(cpc);
      KOUT(INF) << cpc.cmd();
      KOUT(NON) << "Creating lib: " << mkn::kul::File(cpc.file()).real();
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
      if (AppVars::INSTANCE().nodes()) DistLinker::send(cpc.file());
#endif
    }
    return cpc;
  } else
    KEXCEPTION("Unable to handle artifact: \"" + lang + "\" - type is not in file list");
}
