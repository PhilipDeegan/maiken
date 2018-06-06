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
class Executioner : public Constants {
  friend class Application;

  static CompilerProcessCapture build_exe(const kul::hash::set::String& objects,
                                          const std::string& main,
                                          const std::string& out,
                                          const kul::Dir outD,
                                          Application& app) {
    const std::string& file = main;
    const std::string& fileType = file.substr(file.rfind(".") + 1);

    if (!(*app.files().find(fileType)).second.count(STR_COMPILER))
      KEXIT(1, "No compiler found for filetype " + fileType);
    if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() &&
        !app.libraries().empty()) {
      KOUT(NON) << "LIBRARIES";
      for (const std::string& s : app.libraries()) KOUT(NON) << "\t" << s;
    }
    if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() &&
        !app.libraryPaths().empty()) {
      KOUT(NON) << "LIBRARY PATHS";
      for (const std::string& s : app.libraryPaths()) KOUT(NON) << "\t" << s;
    }
    try {
      std::string linker = app.fs[fileType][STR_LINKER];
      std::string linkEnd;
      if (app.ro) linkEnd = AppVars::INSTANCE().linker();
      if (!AppVars::INSTANCE().allinker().empty())
        linkEnd += " " + AppVars::INSTANCE().allinker();
      if (!app.lnk.empty()) linkEnd += " " + app.lnk;
      if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() &&
          linkEnd.size())
        KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
      std::string bin(AppVars::INSTANCE().dryRun()
                          ? kul::File(outD.join(out)).esc()
                          : kul::File(outD.join(out)).escm());
      std::vector<std::string> obV;
      for (const auto& o : objects) obV.emplace_back(o);
      const std::string& base(Compilers::INSTANCE().base(
          (*(*app.files().find(fileType)).second.find(STR_COMPILER)).second));
      if (app.cLnk.count(base)) linkEnd += " " + app.cLnk[base];
      auto* comp = Compilers::INSTANCE().get(base);
      auto linkOpt(comp->linkerOptimizationBin(AppVars::INSTANCE().optimise()));
      if (!linkOpt.empty()) linker += " " + linkOpt;
      auto linkDbg(comp->linkerDebugBin(AppVars::INSTANCE().debug()));
      if (!linkDbg.empty()) linker += " " + linkDbg;
      const CompilerProcessCapture& cpc = comp->buildExecutable(
          linker, linkEnd, obV, app.libraries(), app.libraryPaths(), bin, app.m,
          AppVars::INSTANCE().dryRun());
      if (AppVars::INSTANCE().dryRun())
        KOUT(NON) << cpc.cmd();
      else {
        app.checkErrors(cpc);
        KOUT(INF) << cpc.cmd();
        KOUT(NON) << "Creating bin: " << kul::File(cpc.file()).real();
      }
      return cpc;
    } catch (const CompilerNotFoundException& e) {
      KEXCEPTION("UNSUPPORTED COMPILER EXCEPTION");
    }
  }
};
}

void maiken::Application::buildExecutable(const kul::hash::set::String& objects)
    KTHROW(kul::Exception) {
  KLOG(INF) << main;
  const std::string& file = main;
  const std::string& fileType = file.substr(file.rfind(".") + 1);
  if (fs.count(fileType) == 0)
    KEXIT(1, "Unable to handle artifact: \"" + file +
                 "\" - type is not in file list");

  const std::string oType(
      "." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
  kul::Dir objD(buildDir().join("obj"));

  const std::string& name(out.empty() ? project().root()[STR_NAME].Scalar()
                                      : out);

  std::vector<std::pair<std::string, std::string>> source_objects;
  kul::hash::set::String cobjects = objects;

  const kul::File source(main);
  std::stringstream ss, os;
  ss << std::hex << std::hash<std::string>()(source.real());
  os << ss.str() << "-" << source.name() << oType;
  kul::File object(os.str(), objD);
  KLOG(INF) << object;
  source_objects.emplace_back(std::make_pair(
      AppVars::INSTANCE().dryRun() ? source.esc() : source.escm(),
      AppVars::INSTANCE().dryRun() ? object.esc() : object.escm()));

  std::vector<kul::File> cacheFiles;
  compile(source_objects, cobjects, cacheFiles);
  Executioner::build_exe(cobjects, main, name,
                         kul::Dir(inst ? inst.real() : buildDir()), *this);
  object.mv(kul::File(object.name().substr(0, object.name().rfind(".")), objD));
}

void maiken::Application::buildTest(const kul::hash::set::String& objects)
    KTHROW(kul::Exception) {
  const std::string oType(
      "." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
  kul::Dir objD(buildDir().join("obj"));

  kul::Dir testsD(buildDir().join("test"));
  for (const auto& p : tests) {
    const std::string& file = p.first;
    const std::string& fileType = file.substr(file.rfind(".") + 1);
    if (fs.count(fileType) == 0) continue;

    if (!testsD) testsD.mk();
    std::vector<std::pair<std::string, std::string>> source_objects;
    kul::hash::set::String cobjects = objects;

    const kul::File source(p.first);

    std::stringstream exe_ss, exe_os;
    exe_ss << std::hex << std::hash<std::string>()(source.real());
    exe_os << exe_ss.str() << "-" << source.real() << source.name();
    kul::File exe(exe_os.str(), objD);

    std::stringstream ss, os;
    ss << std::hex << std::hash<std::string>()(source.real());
    os << ss.str() << "-" << source.name() << oType;
    kul::File object(os.str(), objD);
    source_objects.emplace_back(std::make_pair(
        AppVars::INSTANCE().dryRun() ? source.esc() : source.escm(),
        AppVars::INSTANCE().dryRun() ? object.esc() : object.escm()));
    std::vector<kul::File> cacheFiles;
    compile(source_objects, cobjects, cacheFiles);
    Executioner::build_exe(cobjects, p.first, exe.real(), testsD, *this);
    object.mv(kul::File(object.name().substr(0, object.name().rfind(".")), objD));
  }
}
