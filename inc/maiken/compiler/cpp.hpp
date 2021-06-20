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
  Exception(char const* f, int const l, std::string s) : kul::Exception(f, l, s) {}
};


class CCompiler : public Compiler {
 protected:
  CCompiler(){}
  CCompiler(int const& v) : Compiler(v) {}

 public:
  virtual ~CCompiler() {}
  virtual std::string cc() const = 0;
  virtual std::string cxx() const = 0;
  virtual std::string defaultSharedLib(std::string const& lib) const;
  virtual std::string sharedLib(Application const& app, std::string const& lib) const;
  virtual std::string staticLib(std::string const& lib) const = 0;
  bool sourceIsBin() const override { return false; }

  std::string const oType() const { return AppVars::INSTANCE().envVars().at("MKN_OBJ"); }
  std::string const oStar() const { return "*." + oType(); }

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
  GccCompiler(){}
  GccCompiler(int const& v);
  virtual ~GccCompiler() {}

  std::string staticLib(std::string const& lib) const override { return "lib" + lib + ".a"; }

  virtual std::string cc() const override { return CC("gcc"); }
  virtual std::string cxx() const override { return CXX("g++"); }

  CompilerProcessCapture compileSource(CompileDAO& dao) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(kul::Exception) override;

  virtual void preCompileHeader(std::vector<std::string> const& incs,
                                std::vector<std::string> const& args, std::string const& in,
                                std::string const& out, bool dryRun = false) const
      KTHROW(kul::Exception) override;

  void rpathing(maiken::Application const& app, kul::Process& p, kul::File const& out,
                std::vector<std::string> const& libs,
                std::vector<std::string> const& libPaths) const;
};

class ClangCompiler : public GccCompiler {
 public:
  ClangCompiler(int const& v = 0) : GccCompiler(v) {}
  std::string cc() const override { return CC("clang"); }
  std::string cxx() const override { return CXX("clang++"); }
};

class HccCompiler : public GccCompiler {
 public:
  HccCompiler(int const& v = 0) : GccCompiler(v) {}
  std::string cc() const override { return CC("hcc"); }
  std::string cxx() const override { return CXX("h++"); }
};

class IntelCompiler : public GccCompiler {
 public:
  IntelCompiler(int const& v = 0) : GccCompiler(v) {}
  std::string cc() const override { return CC("icc"); }
  std::string cxx() const override { return CXX("icpc"); }
};

class PGCC_Compiler : public GccCompiler {
 public:
  PGCC_Compiler();
  std::string cc() const override { return CC("pgcc"); }
  std::string cxx() const override { return CXX("pgc++"); }
};

class WINCompiler : public CCompiler {
 protected:
 public:
  WINCompiler(int const& v = 0);
  virtual ~WINCompiler() {}

  std::string cc() const override { return CC("cl"); }
  std::string cxx() const override { return CXX("cl"); }

  std::string staticLib(std::string const& lib) const override { return lib + ".lib"; }

  CompilerProcessCapture compileSource(CompileDAO& dao) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(kul::Exception) override;

  virtual void preCompileHeader(std::vector<std::string> const&, std::vector<std::string> const&,
                                std::string const&, std::string const&, bool = false) const
      KTHROW(kul::Exception) override {
    KEXCEPTION("Method is not implemented");
  }
  // CCompiler_Type type() const override { return CCompiler_Type::WIN; }
};
}  // namespace cpp
}  // namespace maiken
#endif /* _MAIKEN_CODE_CPP_HPP_ */
