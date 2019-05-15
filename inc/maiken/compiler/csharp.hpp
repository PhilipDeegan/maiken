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
#ifndef _MAIKEN_CODE_CSHARP_HPP_
#define _MAIKEN_CODE_CSHARP_HPP_

#include "maiken/compiler.hpp"

namespace maiken {
namespace csharp {

class Exception : public kul::Exception {
 public:
  Exception(const char *f, const int l, std::string s) : kul::Exception(f, l, s) {}
};

class WINCompiler : public Compiler {
 public:
  WINCompiler(const int &v = 0) : Compiler(v) {}
  bool sourceIsBin() const override { return true; }

  CompilerProcessCapture buildExecutable(const std::string &linker, const std::string &linkerEnd,
                                         const std::vector<std::string> &objects,
                                         const std::vector<std::string> &libs,
                                         const std::vector<std::string> &libPaths,
                                         const std::string &out, const compiler::Mode &mode,
                                         bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(const std::string &linker, const std::string &linkerEnd,
                                      const std::vector<std::string> &objects,
                                      const std::vector<std::string> &libs,
                                      const std::vector<std::string> &libPaths,
                                      const kul::File &out, const compiler::Mode &mode,
                                      bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture compileSource(const maiken::Application &app,
                                       const std::string &compiler,
                                       const std::vector<std::string> &args,
                                       const std::vector<std::string> &incs, const std::string &in,
                                       const std::string &out, const compiler::Mode &mode,
                                       bool dryRun = false) const KTHROW(kul::Exception) override {
    (void)compiler;
    (void)args;
    (void)incs;
    (void)in;
    (void)out;
    (void)mode;
    (void)dryRun;
    KEXCEPTION("Method compileSource is not implemented in C Sharp");
  }
  virtual void preCompileHeader(const std::vector<std::string> &incs,
                                const std::vector<std::string> &args, const std::string &in,
                                const std::string &out, bool dryRun = false) const
      KTHROW(kul::Exception) override {
    (void)incs;
    (void)args;
    (void)in;
    (void)out;
    (void)dryRun;
    KEXCEPTION("Method preCompileHeader is not implemented in C Sharp");
  }
};
}  // namespace csharp
}  // namespace maiken
#endif /* _MAIKEN_CODE_CSHARP_HPP_ */
