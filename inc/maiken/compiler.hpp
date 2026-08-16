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
#ifndef MAIKEN_COMPILER_HPP
#define MAIKEN_COMPILER_HPP

#include "mkn/kul/lang/compiler.hpp"

namespace maiken {
class MKN_KUL_PUBLISH Application;

using Context = mkn::kul::lang::Context;
using ContextState = mkn::kul::lang::ContextState;
using CompilerState = mkn::kul::lang::CompilerState;
using AbstractCompilerInput = mkn::kul::lang::AbstractCompilerInput;
using IncludeInput = mkn::kul::lang::IncludeInput;
using LibPathInput = mkn::kul::lang::LibPathInput;
using LibInput = mkn::kul::lang::LibInput;
using LinkPrefixInput = mkn::kul::lang::LinkPrefixInput;
using BuildModeInput = mkn::kul::lang::BuildModeInput;
using CompilationInfoInput = mkn::kul::lang::CompilationInfoInput;
using CompilationInfo = mkn::kul::lang::CompilationInfo;
using CompileCommand = mkn::kul::lang::CompileCommand;
using CompileEnv = mkn::kul::lang::CompileEnv;
using Compiler = mkn::kul::lang::Compiler;
using CompilerProcessCapture = mkn::kul::lang::ProcessCapture;
using CompileDAO = mkn::kul::lang::CompileDAO;
using LinkDAO = mkn::kul::lang::LinkDAO;

namespace compiler {
using Mode = mkn::kul::lang::Mode;
using mkn::kul::lang::mode_from;
}  // namespace compiler

struct CompilationUnit {
  CompilationUnit(maiken::Application const& app, Compiler const* comp, std::string const& compiler,
                  std::vector<std::string> const& args, std::string const& in,
                  std::string const& out, compiler::Mode const& mode, bool dryRun)
      : app(app),
        comp(comp),
        compiler(compiler),
        args(args),
        in(in),
        out(out),
        mode(mode),
        dryRun(dryRun) {}

  CompilerProcessCapture compile() const KTHROW(mkn::kul::Exception);

  std::string compileString() const KTHROW(mkn::kul::Exception);

  // Defined once Application is complete (needs app.context()).
  CompileDAO dao() const;

  maiken::Application const& app;
  Compiler const* comp;
  std::string const compiler;
  std::vector<std::string> const args;
  std::string const in;
  std::string const out;
  compiler::Mode const mode;
  bool const dryRun;
};
}  // namespace maiken
#endif /* MAIKEN_COMPILER_HPP */
