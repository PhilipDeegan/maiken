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

class Exception : public kul::Exception
{
public:
  Exception(const char* f, const int l, std::string s)
    : kul::Exception(f, l, s)
  {}
};

class CCompiler : public Compiler
{
protected:
  CCompiler(const int& v)
    : Compiler(v)
  {}

public:
  virtual ~CCompiler() {}
  virtual const std::string cc() const = 0;
  virtual const std::string cxx() const = 0;
  virtual const std::string sharedLib(const std::string& lib) const = 0;
  virtual const std::string staticLib(const std::string& lib) const = 0;
  bool sourceIsBin() const override { return false; }

  const std::string oType(const std::vector<std::string>& objs) const
  {
    size_t i = 0;
    std::string most("o");
    std::unordered_map<std::string, size_t> keys;
    for (const auto& s : objs) {
      std::string t(s.substr(s.rfind(".") + 1));
      if (!keys.count(t))
        keys[t] = 0;
      keys[t]++;
    }
    for (const auto& p : keys) {
      if (p.second > i) {
        i = p.second;
        most = p.first;
      }
    }
    return most;
  }
  const std::string oStar(const std::vector<std::string>& objs) const
  {
    return "*." + oType(objs);
  }
};

class GccCompiler : public CCompiler
{
public:
  GccCompiler(const int& v = 0)
    : CCompiler(v)
  {
    optimise.insert({ { 0, "" },
                      { 1, "-O1" },
                      { 2, "-O2" },
                      { 3, "-O3" },
                      { 4, "-O3 -funroll-loops" },
                      { 5, "-O3 -funroll-loops" },
                      { 6, "-O3 -funroll-loops" },
                      { 7, "-O3 -funroll-loops" },
                      { 8, "-O3 -funroll-loops" },
                      { 9, "-O3 -funroll-loops -march=native" } });
  }
  const std::string sharedLib(const std::string& lib) const override
  {
    return "lib" + lib + ".so";
  }
  const std::string staticLib(const std::string& lib) const override
  {
    return "lib" + lib + ".a";
  }

  virtual const std::string cc() const override { return "gcc"; }
  virtual const std::string cxx() const override { return "g++"; }

  CompilerProcessCapture buildExecutable(
    const std::string& linker,
    const std::string& linkerEnd,
    const std::vector<std::string>& objects,
    const std::vector<std::string>& libs,
    const std::vector<std::string>& libPaths,
    const std::string& out,
    const compiler::Mode& mode,
    bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(const std::string& linker,
                                      const std::string& linkerEnd,
                                      const std::vector<std::string>& objects,
                                      const std::vector<std::string>& libs,
                                      const std::vector<std::string>& libPaths,
                                      const kul::File& out,
                                      const compiler::Mode& mode,
                                      bool dryRun = false) const
    KTHROW(kul::Exception) override;

  CompilerProcessCapture compileSource(const std::string& compiler,
                                       const std::vector<std::string>& args,
                                       const std::vector<std::string>& incs,
                                       const std::string& in,
                                       const std::string& out,
                                       const compiler::Mode& mode,
                                       bool dryRun = false) const
    KTHROW(kul::Exception) override;

  virtual void preCompileHeader(const std::vector<std::string>& incs,
                                const std::vector<std::string>& args,
                                const std::string& in,
                                const std::string& out,
                                bool dryRun = false) const
    KTHROW(kul::Exception) override;
};

class ClangCompiler : public GccCompiler
{
public:
  ClangCompiler(const int& v = 0)
    : GccCompiler(v)
  {}
  virtual const std::string cc() const override { return "clang"; }
  virtual const std::string cxx() const override { return "clang++"; }
};

class HccCompiler : public GccCompiler
{
public:
  HccCompiler(const int& v = 0)
    : GccCompiler(v)
  {}
  virtual const std::string cc() const override { return "hcc"; }
  virtual const std::string cxx() const override { return "h++"; }
};

class IntelCompiler : public GccCompiler
{
public:
  IntelCompiler(const int& v = 0)
    : GccCompiler(v)
  {}
  virtual const std::string cc() const override { return "icc"; }
  virtual const std::string cxx() const override { return "icpc"; }
};

class WINCompiler : public CCompiler
{
protected:
public:
  WINCompiler(const int& v = 0)
    : CCompiler(v)
  {
    optimise.insert({ { 0, "" },
                      { 1, "-O1" },
                      { 2, "-O2" },
                      { 3, "-O3" },
                      { 4, "-O3 -funroll-loops" },
                      { 5, "-O3 -funroll-loops" },
                      { 6, "-O3 -funroll-loops" },
                      { 7, "-O3 -funroll-loops" },
                      { 8, "-O3 -funroll-loops" },
                      { 9, "-O3 -funroll-loops -march=native" } });
  }
  virtual const std::string cc() const override { return "cl"; }
  virtual const std::string cxx() const override { return "cl"; }
  const std::string sharedLib(const std::string& lib) const override
  {
    return lib + ".dll";
  }
  const std::string staticLib(const std::string& lib) const override
  {
    return lib + ".lib";
  }

  CompilerProcessCapture buildExecutable(
    const std::string& linker,
    const std::string& linkerEnd,
    const std::vector<std::string>& objects,
    const std::vector<std::string>& libs,
    const std::vector<std::string>& libPaths,
    const std::string& out,
    const compiler::Mode& mode,
    bool dryRun = false) const KTHROW(kul::Exception) override;

  CompilerProcessCapture buildLibrary(const std::string& linker,
                                      const std::string& linkerEnd,
                                      const std::vector<std::string>& objects,
                                      const std::vector<std::string>& libs,
                                      const std::vector<std::string>& libPaths,
                                      const kul::File& out,
                                      const compiler::Mode& mode,
                                      bool dryRun = false) const
    KTHROW(kul::Exception) override;

  CompilerProcessCapture compileSource(const std::string& compiler,
                                       const std::vector<std::string>& args,
                                       const std::vector<std::string>& incs,
                                       const std::string& in,
                                       const std::string& out,
                                       const compiler::Mode& mode,
                                       bool dryRun = false) const
    KTHROW(kul::Exception) override;

  virtual void preCompileHeader(const std::vector<std::string>& incs,
                                const std::vector<std::string>& args,
                                const std::string& in,
                                const std::string& out,
                                bool dryRun = false) const
    KTHROW(kul::Exception) override
  {
    KEXCEPTION("Method is not implemented");
  }
};

} // namespace cpp
} // namespace maiken
#endif /* _MAIKEN_CODE_CPP_HPP_ */
