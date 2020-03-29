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
#ifndef _MAIKEN_CODE_CPP_HPP_
#define _MAIKEN_CODE_CPP_HPP_

#include <unordered_map>

#include "maiken/compiler.hpp"

namespace maiken {

namespace cpp {

class Exception : public kul::Exception {
 public:
  Exception(const char *f, const int l, std::string s) : kul::Exception(f, l, s) {}
};

enum class CCompiler_Type : uint16_t { NON = 0, GCC = 1, CLANG = 2, ICC = 3, HCC = 4, WIN = 5 };

class CCompiler : public Compiler {
 protected:
  CCompiler(const int &v) : Compiler(v) {}

 public:
  virtual ~CCompiler() {}
  virtual std::string cc() const = 0;
  virtual std::string cxx() const = 0;
  virtual std::string defaultSharedLib(std::string const &lib) const;
  virtual std::string sharedLib(Application const &app, std::string const &lib) const;
  virtual std::string staticLib(std::string const &lib) const = 0;
  bool sourceIsBin() const override { return false; }

  const std::string oType(std::vector<std::string> const &objs) const {
    size_t i = 0;
    std::string most("o");
    std::unordered_map<std::string, size_t> keys;
    for (const auto &s : objs) {
      std::string t(s.substr(s.rfind(".") + 1));
      if (!keys.count(t)) keys[t] = 0;
      keys[t]++;
    }
    for (const auto &p : keys) {
      if (p.second > i) {
        i = p.second;
        most = p.first;
      }
    }
    return most;
  }
  const std::string oStar(std::vector<std::string> const &objs) const { return "*." + oType(objs); }

  virtual CCompiler_Type type() const = 0;

  static std::string CC(std::string deFault) {
    return kul::env::EXISTS("CC") ? kul::env::GET("CC") : deFault;
  }
  static std::string CXX(std::string deFault) {
    return kul::env::EXISTS("CXX") ? kul::env::GET("CXX") : deFault;
  }
  static std::string LD(std::string deFault) {
    return kul::env::EXISTS("LD") ? kul::env::GET("LD") : deFault;
  }
};

class GccCompiler : public CCompiler {
 public:
  GccCompiler(const int &v = 0);

  std::string staticLib(std::string const &lib) const override { return "lib" + lib + ".a"; }

  virtual std::string cc() const override { return CC("gcc"); }
  virtual std::string cxx() const override { return CXX("g++"); }

  CompilerProcessCapture buildExecutable(maiken::Application const &app, std::string const &linker,
                                         std::string const &linkerEnd,
                                         std::vector<std::string> const &objects,
                                         std::vector<std::string> const &libs,
                                         std::vector<std::string> const &libPaths,
                                         std::string const &out, const compiler::Mode &mode,
                                         bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(maiken::Application const &app, std::string const &linker,
                                      std::string const &linkerEnd,
                                      std::vector<std::string> const &objects,
                                      std::vector<std::string> const &libs,
                                      std::vector<std::string> const &libPaths,
                                      const kul::File &out, const compiler::Mode &mode,
                                      bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture compileSource(const maiken::Application &app, std::string const &compiler,
                                       std::vector<std::string> const &args,
                                       std::vector<std::string> const &incs, std::string const &in,
                                       std::string const &out, const compiler::Mode &mode,
                                       bool dryRun = false) const KTHROW(kul::Exception) override;

  virtual void preCompileHeader(std::vector<std::string> const &incs,
                                std::vector<std::string> const &args, std::string const &in,
                                std::string const &out, bool dryRun = false) const
      KTHROW(kul::Exception) override;

  CCompiler_Type type() const override { return CCompiler_Type::GCC; }

  void rpathing(maiken::Application const &app, kul::Process &p, const kul::File &out,
                std::vector<std::string> const &libs,
                std::vector<std::string> const &libPaths) const;
};

class ClangCompiler : public GccCompiler {
 public:
  ClangCompiler(const int &v = 0);
  std::string cc() const override { return CC("clang"); }
  std::string cxx() const override { return CXX("clang++"); }
  CCompiler_Type type() const override { return CCompiler_Type::CLANG; }
};

class HccCompiler : public GccCompiler {
 public:
  HccCompiler(const int &v = 0) : GccCompiler(v) {}
  std::string cc() const override { return CC("hcc"); }
  std::string cxx() const override { return CXX("h++"); }
  CCompiler_Type type() const override { return CCompiler_Type::HCC; }
};

class IntelCompiler : public GccCompiler {
 public:
  IntelCompiler(const int &v = 0);
  std::string cc() const override { return CC("icc"); }
  std::string cxx() const override { return CXX("icpc"); }
  CCompiler_Type type() const override { return CCompiler_Type::ICC; }
};

class WINCompiler : public CCompiler {
 protected:
 public:
  WINCompiler(const int &v = 0);
  std::string cc() const override { return CC("cl"); }
  std::string cxx() const override { return CXX("cl"); }

  std::string staticLib(std::string const &lib) const override { return lib + ".lib"; }

  CompilerProcessCapture buildExecutable(maiken::Application const &app, std::string const &linker,
                                         std::string const &linkerEnd,
                                         std::vector<std::string> const &objects,
                                         std::vector<std::string> const &libs,
                                         std::vector<std::string> const &libPaths,
                                         std::string const &out, const compiler::Mode &mode,
                                         bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(maiken::Application const &app, std::string const &linker,
                                      std::string const &linkerEnd,
                                      std::vector<std::string> const &objects,
                                      std::vector<std::string> const &libs,
                                      std::vector<std::string> const &libPaths,
                                      const kul::File &out, const compiler::Mode &mode,
                                      bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture compileSource(const maiken::Application &app, std::string const &compiler,
                                       std::vector<std::string> const &args,
                                       std::vector<std::string> const &incs, std::string const &in,
                                       std::string const &out, const compiler::Mode &mode,
                                       bool dryRun = false) const KTHROW(kul::Exception) override;

  virtual void preCompileHeader(std::vector<std::string> const &incs,
                                std::vector<std::string> const &args, std::string const &in,
                                std::string const &out, bool dryRun = false) const
      KTHROW(kul::Exception) override {
    (void)incs;
    (void)args;
    (void)in;
    (void)out;
    (void)dryRun;
    KEXCEPTION("Method is not implemented");
  }
  CCompiler_Type type() const override { return CCompiler_Type::WIN; }
};
}  // namespace cpp
}  // namespace maiken
#endif /* _MAIKEN_CODE_CPP_HPP_ */
