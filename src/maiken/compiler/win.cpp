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

maiken::cpp::WINCompiler::WINCompiler(int const& v) : CCompiler(v) {
  m_optimise_c.insert({{0, "-Od"},
                       {1, "-O1"},
                       {2, "-O2"},
                       {3, "-O2"},
                       {4, "-O2"},
                       {5, "-O2"},
                       {6, "-O2"},
                       {7, "-O2"},
                       {8, "-O2"},
                       {9, "-Ox -GA -Gw -Zc:inline -EHsc"}});
  m_debug_c.insert({{0, "-DNDEBUG"},
                    {1, ""},
                    {2, ""},
                    {3, ""},
                    {4, ""},
                    {5, ""},
                    {6, ""},
                    {7, ""},
                    {8, ""},
                    {9, "-Z7"}});
  m_optimise_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_optimise_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_debug_l_bin.insert({{0, ""},
                        {1, ""},
                        {2, ""},
                        {3, ""},
                        {4, ""},
                        {5, ""},
                        {6, ""},
                        {7, ""},
                        {8, ""},
                        {9, "-DEBUG"}});
  m_debug_l_lib.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_warn_c.insert({{0, "-W0"},
                   {1, "-W1"},
                   {2, "-W2"},
                   {3, "-W3"},
                   {4, "-W4"},
                   {5, "-Wall"},
                   {6, "-Wall"},
                   {7, "-Wall"},
                   {8, "-Wall"},
                   {9, "-Wall"}});
}

maiken::CompilerProcessCapture maiken::cpp::WINCompiler::buildExecutable(LinkDAO& dao) const
    KTHROW(mkn::kul::Exception) {
  auto& app = dao.app;
  auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
  auto& dryRun = dao.dryRun;
  auto &linker = dao.linker, &linkerEnd = dao.linkerEnd, &out = dao.out;

  std::vector<std::string> fobjects;
  mkn::kul::hash::set::String dirs;
  for (auto const& d : dao.stars) {
    dirs.insert(d.real());
    for (auto const& f : d.files()) fobjects.emplace_back(f.escm());
  }

  std::string exe = out + ".exe";

  std::string cmd = LD(linker);
  std::vector<std::string> bits;
  if (cmd == linker && linker.find(" ") != std::string::npos) {
    bits = mkn::kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  mkn::kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  p.arg("-OUT:\"" + exe + "\"").arg("-nologo");
  for (std::string const& path : libPaths) p.arg("-LIBPATH:\"" + path + "\"");
  for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(), d).escm());
  for (std::string const& o : objects) p << mkn::kul::File(o).escm();
  for (std::string const& lib : libs) p.arg(staticLib(lib));
  for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p.arg(s);

  CompilerProcessCapture pc;
  try {
    if (!dryRun) p.set(app.envVars()).start();
  } catch (const mkn::kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(exe);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture maiken::cpp::WINCompiler::buildLibrary(LinkDAO& dao) const
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
  std::string imp = out.dir().join(staticLib(out.name()));
  if (mode == compiler::Mode::STAT) lib = out.dir().join(staticLib(out.name()));
  std::string cmd = linker;
  if (mode == compiler::Mode::SHAR) cmd = LD(linker);
  std::vector<std::string> bits;
  if (cmd == linker && linker.find(" ") != std::string::npos) {
    bits = mkn::kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  mkn::kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  p.arg("-nologo");
  if (mode == compiler::Mode::STAT) p.arg("-LTCG");
  p.arg("-OUT:\"" + lib + "\"");
  if (mode == compiler::Mode::SHAR) {
    p.arg("-IMPLIB:\"" + imp + "\"").arg("-DLL");
    for (std::string const& path : libPaths) p.arg("-LIBPATH:\"" + path + "\"");
    for (std::string const& lib : libs) p.arg(staticLib(lib));
  }
  for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(), d).escm());
  for (std::string const& o : objects) p << mkn::kul::File(o).escm();
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

maiken::CompilerProcessCapture maiken::cpp::WINCompiler::compileSource(CompileDAO& dao) const
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
  for (size_t i = 1; i < bits.size(); i++) p.arg(bits[i]);
  p.arg("-nologo");
  for (auto const& def : app.defines()) p << std::string("-D" + def);
  for (std::string const& s : incs) p.arg("-I\"" + s + "\"");
  for (std::string const& s : args) p.arg(s);
  p.arg("-c").arg("-Fo\"" + out + "\"").arg("\"" + in + "\"");
  CompilerProcessCapture pc;
  if (!mkn::kul::LogMan::INSTANCE().inf()) pc.setProcess(p);

  try {
    if (!dryRun) p.set(app.envVars()).start();
  } catch (mkn::kul::Exception const& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}
