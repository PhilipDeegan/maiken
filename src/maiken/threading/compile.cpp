/**
Copyright (c) 2026, Philip Deegan.
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
#include "maiken/compiler/compilers.hpp"
#include "maiken.hpp"

namespace maiken {

std::vector<std::string> resolve_includes(Application const& app) {
  std::vector<std::string> incs;
  for (auto const& s : app.includes()) {
    mkn::kul::Dir const d(s.first);
    std::string const m = d.escm();
    incs.push_back(m.empty() ? "." : m);
  }
  return incs;
}

CompilationUnit build_compilation_unit(Application const& app, std::vector<std::string> const& incs,
                                       std::pair<Source, std::string> const& p)
    KTHROW(mkn::kul::Exception) {
  std::string const src(p.first.in), obj(p.second);
  std::string const& fileType = src.substr(src.rfind(".") + 1);
  if (!(app.files().count(fileType))) KEXCEPTION("NOOOOOOO ", fileType);
  std::string const& compiler =
      (*(*app.files().find(fileType)).second.find(Constants::STR_COMPILER)).second;
  std::string const& base = Compilers::INSTANCE().base(compiler);
  auto comp = Compilers::INSTANCE().get(compiler);

  // Baked in up front so CompileDAO::args is the complete, ready-to-use flag
  // list - a module reading a real compile command via per_compiler_command
  // doesn't need to know about defines/incs formatting separately.
  std::vector<std::string> args;
  for (auto const& def : app.defines()) args.push_back("-D" + def);
  for (auto const& inc : incs) args.push_back(comp->includeArg(inc));

  if (app.arguments().count(fileType) > 0)
    for (std::string const& o : (*app.arguments().find(fileType)).second)
      for (auto const& s : mkn::kul::cli::asArgs(o)) args.push_back(s);
  for (auto const& s : mkn::kul::cli::asArgs(app.compileArg())) args.push_back(s);
  if (app.compilerArgs().count(base))
    for (auto const& s : mkn::kul::cli::asArgs(app.compilerArgs().at(base))) args.push_back(s);
  // WE CHECK BEFORE USING THIS THAT A COMPILER EXISTS FOR EVERY FILE
  auto compilerFlags = [&args](std::string const& as) {
    for (auto const& s : mkn::kul::cli::asArgs(as)) args.push_back(s);
  };
  if (AppVars::INSTANCE().jargs().count(fileType) > 0)
    compilerFlags((*AppVars::INSTANCE().jargs().find(fileType)).second);
  compilerFlags(AppVars::INSTANCE().args());
  compilerFlags(comp->compilerDebug(AppVars::INSTANCE().debug()));
  compilerFlags(comp->compilerOptimization(AppVars::INSTANCE().optimise()));
  compilerFlags(comp->compilerWarning(AppVars::INSTANCE().warn()));
  compilerFlags(p.first.args);
  return CompilationUnit(app, comp, compiler, args, src, obj, app.mode(),
                         AppVars::INSTANCE().dryRun());
}

std::string CompilationUnit::compileString() const KTHROW(mkn::kul::Exception) {
  mkn::kul::os::PushDir pushd(app.project().dir());
  auto d = dao();
  d.dryRun = true;
  return comp->compileSource(d).cmd();
}

CompilerProcessCapture CompilationUnit::compile() const KTHROW(mkn::kul::Exception) {
  try {
    mkn::kul::os::PushDir pushd(app.project().dir());
    auto d = dao();
    return comp->compileSource(d);
  } catch (std::exception const& e) {
    throw;
  }
}

}  // namespace maiken
