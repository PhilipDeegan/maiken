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
#ifndef _MAIKEN_COMPILER_HPP_
#define _MAIKEN_COMPILER_HPP_

#include "mkn/kul/cli.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/map.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/string.hpp"

#include "maiken/global.hpp"

namespace maiken {
class KUL_PUBLISH Application;

struct CompilationInfo {
  std::string lib_prefix, lib_postfix, lib_ext;

  CompilationInfo()
      : lib_prefix(AppVars::INSTANCE().envVar("MKN_LIB_PRE")),
        lib_ext(AppVars::INSTANCE().envVar("MKN_LIB_EXT")) {}
};

namespace compiler {
enum Mode { NONE = 0, STAT, SHAR };

inline auto mode_from(std::string const& mode){
  if(mode == Constants::STR_SHARED)
      return Mode::SHAR;
  if(mode == Constants::STR_STATIC)
      return Mode::STAT;
  return Mode::NONE;
}
}

struct CompileDAO {
  maiken::Application const& app;
  std::string const &compiler, &in, &out;
  std::vector<std::string> const &args, &incs;
  compiler::Mode const& mode;
  bool dryRun = false;
};
struct LinkDAO {
  maiken::Application const& app;
  std::string const &linker, &linkerEnd, &out;
  std::vector<mkn::kul::Dir> stars;
  std::vector<std::string> const &objects, &libs, &libPaths;
  compiler::Mode const& mode;
  bool dryRun = false;
};

class CompilerProcessCapture : public mkn::kul::ProcessCapture {
 public:
  CompilerProcessCapture() {}
  CompilerProcessCapture(mkn::kul::AProcess& p) : mkn::kul::ProcessCapture(p) {}

  void exception(std::exception_ptr const& e) { ep = e; }
  std::exception_ptr const& exception() const { return ep; }

  void cmd(std::string const& cm) { this->c = cm; }
  std::string const& cmd() const { return c; }

  void file(std::string const& f) { this->f = f; }
  std::string const& file() const { return f; }

 private:
  std::exception_ptr ep;
  std::string c, f;
};

class Compiler {
 protected:
  Compiler(int const& v) : version(v) {}
  int const version;
  std::unordered_map<uint8_t, std::string> m_optimise_c, m_optimise_l_bin, m_optimise_l_lib,
      m_debug_c, m_debug_l_bin, m_debug_l_lib, m_warn_c;

 public:
  virtual ~Compiler() {}
  virtual bool sourceIsBin() const = 0;

  virtual CompilerProcessCapture compileSource(CompileDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  virtual CompilerProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  virtual CompilerProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  virtual void preCompileHeader(std::vector<std::string> const& incs,
                                std::vector<std::string> const& args, std::string const& in,
                                std::string const& out, bool dryRun = false) const
      KTHROW(mkn::kul::Exception) = 0;

  std::string compilerDebug(uint8_t const& key) const {
    return m_debug_c.count(key) ? m_debug_c.at(key) : "";
  }
  std::string compilerOptimization(uint8_t const& key) const {
    return m_optimise_c.count(key) ? m_optimise_c.at(key) : "";
  }
  std::string compilerWarning(uint8_t const& key) const {
    return m_warn_c.count(key) ? m_warn_c.at(key) : "";
  }
  std::string linkerDebugBin(uint8_t const& key) const {
    return m_debug_l_bin.count(key) ? m_debug_l_bin.at(key) : "";
  }
  std::string linkerDebugLib(uint8_t const& key) const {
    return m_debug_l_lib.count(key) ? m_debug_l_lib.at(key) : "";
  }
  std::string linkerOptimizationBin(uint8_t const& key) const {
    return m_optimise_l_bin.count(key) ? m_optimise_l_bin.at(key) : "";
  }
  std::string linkerOptimizationLib(uint8_t const& key) const {
    return m_optimise_l_lib.count(key) ? m_optimise_l_lib.at(key) : "";
  }
};

struct CompilationUnit {
  CompilationUnit(maiken::Application const& app, Compiler const* comp, std::string const& compiler,
                  std::vector<std::string> const& args, std::vector<std::string> const& incs,
                  std::string const& in, std::string const& out, compiler::Mode const& mode,
                  bool dryRun)
      : app(app),
        comp(comp),
        compiler(compiler),
        args(args),
        incs(incs),
        in(in),
        out(out),
        mode(mode),
        dryRun(dryRun) {}

  CompilerProcessCapture compile() const KTHROW(mkn::kul::Exception);

  std::string compileString() const KTHROW(mkn::kul::Exception);

  maiken::Application const& app;
  Compiler const* comp;
  std::string const compiler;
  std::vector<std::string> const args;
  std::vector<std::string> const incs;
  std::string const in;
  std::string const out;
  compiler::Mode const mode;
  bool const dryRun;
};
}  // namespace maiken
#endif /* _MAIKEN_COMPILER_HPP_ */
