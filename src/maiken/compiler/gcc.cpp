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

maiken::cpp::GccCompiler::GccCompiler(int const& v) : CCompiler(v) {
  m_optimise_c.insert({{0, ""},
                       {1, "-O1"},
                       {2, "-O2"},
                       {3, "-O3"},
                       {4, "-O3 -funroll-loops"},
                       {5, "-O3 -funroll-loops"},
                       {6, "-O3 -funroll-loops"},
                       {7, "-O3 -funroll-loops"},
                       {8, "-O3 -funroll-loops"},
                       {9, "-O3 -funroll-loops -march=native -mtune=native"}});
  m_debug_c.insert({{0, "-DNDEBUG"},
                    {1, "-g1"},
                    {2, "-g2"},
                    {3, "-g3"},
                    {4, "-g3"},
                    {5, "-g3"},
                    {6, "-g3"},
                    {7, "-g3"},
                    {8, "-g3"},
                    {9, "-g3 -pg"}});
  m_optimise_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_optimise_l_lib.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_debug_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_debug_l_lib.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_warn_c.insert({{0, "-w"},
                   {1, "-Wall"},
                   {2, "-Wall"},
                   {3, "-Wall"},
                   {4, "-Wall"},
                   {5, "-Wall"},
                   {6, "-Wall"},
                   {7, "-Wall"},
                   {8, "-Wall -Wextra"},
                   {9, "-Wall -Wextra -pedantic -Werror"}});
}

void maiken::cpp::GccCompiler::rpathing(maiken::Application const& app, mkn::kul::Process& p,
                                        [[maybe_unused]] mkn::kul::File const& out,
                                        std::vector<std::string> const& libs,
                                        std::vector<std::string> const& libPaths) const {
  std::unordered_set<std::string> rpaths;
  for (std::string const& path : libPaths) {
    for (std::string const& lib : libs) {
      mkn::kul::File lib_file(sharedLib(app, lib), path), def(defaultSharedLib(lib), path);
      if (!lib_file && def) lib_file = def;
      if (lib_file) rpaths.emplace(lib_file.dir().real());
    }
  }
  for (auto const& rpath : rpaths) {
    mkn::kul::Dir path(rpath);
    std::stringstream loader;
#if defined(__APPLE__)
    loader << "-Wl,-rpath," << path.esc();
    mkn::kul::File tmp_out(out);
    tmp_out.mk();
    loader << " -Wl,-rpath,@loader_path/" << tmp_out.relative(path);
    tmp_out.rm();
#else
    loader << "-Wl,-rpath=" << path.esc();
#endif
    p << loader.str();
  }
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::buildExecutable(LinkDAO& dao) const
    KTHROW(mkn::kul::Exception) {
  auto& app = dao.app;
  auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
  auto& dryRun = dao.dryRun;
  auto &linker = dao.linker, &linkerEnd = dao.linkerEnd, &out = dao.out;
  auto& mode = dao.mode;

  std::vector<std::string> fobjects;
  mkn::kul::hash::set::String dirs;
  for (auto const& d : dao.stars) {
    dirs.insert(d.real());
    for (auto const& f : d.files()) fobjects.emplace_back(f.escm());
  }

  std::string cmd = LD(linker);
  std::vector<std::string> bits;
  if (cmd == linker && linker.find(" ") != std::string::npos) {
    bits = mkn::kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  mkn::kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  for (std::string const& path : libPaths) p.arg("-L" + path);
  if (mode == compiler::Mode::STAT) p.arg("-static");
  {
    auto ll(mkn::kul::env::GET("MKN_LIB_LINK_LIB"));
    if ((ll.size())) {
      if (mode == compiler::Mode::SHAR || mode == compiler::Mode::NONE) {
        mkn::kul::File file(out);
        rpathing(app, p, file, libs, libPaths);
      }
    }
  }
  std::string exe = out;
  if (KTOSTRING(__MKN_KUL_OS__) == std::string("win")) exe += ".exe";
  p.arg("-o").arg(exe);
  for (std::string const& d : dirs) p << mkn::kul::File(oStar(), d).escm();
  for (std::string const& o : objects) p << mkn::kul::File(o).escm();
  for (std::string const& lib : libs) p.arg("-l" + lib);
  for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p << s;

  CompilerProcessCapture pc;
  try {
    if (!dryRun) p.set(app.envVars()).start();
  } catch (const mkn::kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::buildLibrary(LinkDAO& dao) const
    KTHROW(mkn::kul::Exception) {
  auto& app = dao.app;
  auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
  auto& dryRun = dao.dryRun;
  auto &linker = dao.linker, &linkerEnd = dao.linkerEnd;
  auto& mode = dao.mode;

  mkn::kul::File out(dao.out);

  std::vector<std::string> fobjects;
  mkn::kul::hash::set::String dirs;
  for (auto const& d : dao.stars) {
    dirs.insert(d.real());
    for (auto const& f : d.files()) fobjects.emplace_back(f.escm());
  }

  std::string lib = out.dir().join(sharedLib(app, out.name()));

  if (mode == compiler::Mode::STAT) lib = out.dir().join(staticLib(out.name()));
  lib = mkn::kul::File(lib).esc();
  std::string cmd = linker;
  if (mode == compiler::Mode::SHAR) cmd = LD(linker);
  std::vector<std::string> bits;
  if (cmd == linker && linker.find(" ") != std::string::npos) {
    bits = mkn::kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  mkn::kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  if (mode == compiler::Mode::SHAR) p.arg("-shared").arg("-o");
  p.arg(lib);
  for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(), d).escm());
  for (std::string const& o : objects) p << mkn::kul::File(o).escm();

  {
    auto ll(mkn::kul::env::GET("MKN_LIB_LINK_LIB"));
    if (ll.size() && mode == compiler::Mode::SHAR) {
      uint16_t llv = mkn::kul::String::UINT16(ll);
      for (std::string const& path : libPaths) p.arg("-L" + path);
      if (llv == 1) {
        for (std::string const& lib : libs) p.arg("-l" + lib);
        rpathing(app, p, out, libs, libPaths);
      } else {
        for (std::string const& path : libPaths) {
          for (std::string const& lib : libs) {
            mkn::kul::File lib_file(sharedLib(app, lib), path);
            if (lib_file) p << lib_file.escm();
          }
        }
      }
#if defined(__APPLE__)
      std::stringstream loader;
      loader << "-Wl,-install_name,@rpath/" << mkn::kul::File(lib).name();
      p << loader.str();
#endif
    }
  }
  for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p.arg(s);
  CompilerProcessCapture pc;
  try {
    if (!dryRun) p.set(app.envVars()).start();
  } catch (const mkn::kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(lib);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::compileSource(CompileDAO& dao) const
    KTHROW(mkn::kul::Exception) {
  auto& app = dao.app;
  auto &compiler = dao.compiler, &in = dao.in, &out = dao.out;
  auto &args = dao.args, &incs = dao.incs;
  auto& dryRun = dao.dryRun;

  std::string const fileType = in.substr(in.rfind(".") + 1);

  std::string cmd;
  if (mkn::kul::String::NO_CASE_CMP(fileType, "c"))
    cmd = CC(compiler);
  else
    cmd = CXX(compiler);

  std::vector<std::string> bits;
  if (cmd == compiler && compiler.find(" ") != std::string::npos) {
    bits = mkn::kul::cli::asArgs(compiler);
    cmd = bits[0];
  }
  mkn::kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  for (auto const& def : app.defines()) p << std::string("-D" + def);
  for (std::string const& s : incs) {
    mkn::kul::Dir d(s);
    if (d)
      p.arg("-I" + s);
    else
      p.arg("-include " + s);
  }
  for (std::string const& s : args) p.arg(s);
  p.arg("-o").arg(out).arg("-c").arg(in);
  CompilerProcessCapture pc;
  if (!mkn::kul::LogMan::INSTANCE().inf()) pc.setProcess(p);
  try {
    if (!dryRun) p.set(app.envVars()).start();
  } catch (const mkn::kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

void maiken::cpp::GccCompiler::preCompileHeader(std::vector<std::string> const& incs,
                                                std::vector<std::string> const& args,
                                                std::string const& in, std::string const& out,
                                                bool dryRun) const KTHROW(mkn::kul::Exception) {
  if (in.rfind(".") == std::string::npos) KEXCEPT(Exception, "Unknown header type");

  std::string cmd;  // = compiler + " -x";
  std::string h = in.substr(in.rfind(".") + 1);

  if (h.compare("h") == 0)
    cmd = cc() + " -x c-header ";
  else if (h.compare("hpp") == 0)
    cmd = cxx() + " -x c++-header ";
  else
    KEXCEPT(Exception, "Failed to pre-compile header - uknown file type: " + h);
  cmd += in + " ";
  for (std::string const& s : args) cmd += s + " ";
  for (std::string const& s : incs) cmd += "-I" + s + " ";

  cmd += " -o " + out;
  if (!dryRun && mkn::kul::os::exec(cmd) != 0) KEXCEPT(Exception, "Failed to pre-compile header");
}
