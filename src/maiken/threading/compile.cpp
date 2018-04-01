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

maiken::CompilationUnit maiken::ThreadingCompiler::compilationUnit(
    const std::pair<std::string, std::string>& p) const KTHROW(kul::Exception) {
  const std::string src(p.first);
  const std::string obj(p.second);
  const std::string& fileType = src.substr(src.rfind(".") + 1);
  const std::string& compiler =
      (*(*app.files().find(fileType)).second.find(STR_COMPILER)).second;
  const std::string& base = maiken::Compilers::INSTANCE().base(compiler);
  std::vector<std::string> args;
  if (app.arguments().count(fileType) > 0)
    for (const std::string& o : (*app.arguments().find(fileType)).second)
      for (const auto& s : kul::cli::asArgs(o)) args.push_back(s);
  for (const auto& s : kul::cli::asArgs(app.arg)) args.push_back(s);
  if (app.cArg.count(base))
    for (const auto& s : kul::cli::asArgs(app.cArg[base])) args.push_back(s);
  std::string cmd = compiler + " " + AppVars::INSTANCE().args();
  if (AppVars::INSTANCE().jargs().count(fileType) > 0)
    cmd += " " + (*AppVars::INSTANCE().jargs().find(fileType)).second;
  // WE CHECK BEFORE USING THIS THAT A COMPILER EXISTS FOR EVERY FILE
  auto compilerFlags = [&args](const std::string&& as) {
    for (const auto& s : kul::cli::asArgs(as)) args.push_back(s);
  };
  auto comp = Compilers::INSTANCE().get(compiler);
  compilerFlags(comp->compilerDebug(AppVars::INSTANCE().debug()));
  compilerFlags(comp->compilerOptimization(AppVars::INSTANCE().optimise()));
  compilerFlags(comp->compilerWarning(AppVars::INSTANCE().warn()));
  return CompilationUnit(comp, cmd, args, incs, src, obj, app.m,
                         AppVars::INSTANCE().dryRun());
}

maiken::CompilerProcessCapture maiken::CompilationUnit::compile() const
    KTHROW(kul::Exception) {
  try {
    return comp->compileSource(compiler, args, incs, in, out, mode, dryRun);
  } catch (const kul::Exception& e) {
    std::rethrow_exception(std::current_exception());
  } catch (const std::exception& e) {
    std::cerr << __FILE__ << " : " << __LINE__ << e.what() << std::endl;
  }
  KEXCEPT(maiken::Exception, "this shouldn't happen!");
}
