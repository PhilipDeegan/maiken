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
#include "maiken/compiler/cpp.hpp"

maiken::cpp::GccCompiler::GccCompiler(const int& v) : CCompiler(v) {
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
  m_optimise_l_bin.insert({{0, ""},
                           {1, ""},
                           {2, ""},
                           {3, ""},
                           {4, ""},
                           {5, ""},
                           {6, ""},
                           {7, ""},
                           {8, ""},
                           {9, ""}});
  m_optimise_l_bin.insert({{0, ""},
                           {1, ""},
                           {2, ""},
                           {3, ""},
                           {4, ""},
                           {5, ""},
                           {6, ""},
                           {7, ""},
                           {8, ""},
                           {9, ""}});
  m_debug_l_bin.insert({{0, ""},
                        {1, ""},
                        {2, ""},
                        {3, ""},
                        {4, ""},
                        {5, ""},
                        {6, ""},
                        {7, ""},
                        {8, ""},
                        {9, ""}});
  m_debug_l_lib.insert({{0, ""},
                        {1, ""},
                        {2, ""},
                        {3, ""},
                        {4, ""},
                        {5, ""},
                        {6, ""},
                        {7, ""},
                        {8, ""},
                        {9, ""}});
  m_warn_c.insert({{0, "-w"},
                   {1, "-Wall"},
                   {2, "-Wall"},
                   {3, "-Wall"},
                   {4, "-Wall"},
                   {5, "-Wall"},
                   {6, "-Wall"},
                   {7, "-Wall"},
                   {8, "-Wall -Wextra"},
                   {9, "-Wall -Wextra -Werror"}});
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::buildExecutable(
    const std::string& linker, const std::string& linkerEnd,
    const std::vector<std::string>& objects,
    const std::vector<std::string>& libs,
    const std::vector<std::string>& libPaths, const std::string& out,
    const maiken::compiler::Mode& mode, bool dryRun) const
    KTHROW(kul::Exception) {
  kul::hash::set::String dirs;
  for (const auto& o : objects) dirs.insert(kul::File(o).dir().real());

  std::string cmd = linker;
  std::vector<std::string> bits;
  if (linker.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  for (const std::string& path : libPaths) p.arg("-L" + path);
  if (mode == compiler::Mode::STAT) p.arg("-static");
  std::string exe = out;
  if (KTOSTRING(__KUL_OS__) == "win") exe += ".exe";
  p.arg("-o").arg(exe);
  for (const std::string& d : dirs) p.arg(kul::File(oStar(objects), d).escm());
  for (const std::string& lib : libs) p.arg("-l" + lib);
  for (const std::string& s : kul::cli::asArgs(linkerEnd)) p.arg(s);

  CompilerProcessCapture pc;
  try {
    if (!dryRun) p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::buildLibrary(
    const std::string& linker, const std::string& linkerEnd,
    const std::vector<std::string>& objects,
    const std::vector<std::string>& libs,
    const std::vector<std::string>& libPaths, const kul::File& out,
    const maiken::compiler::Mode& mode, bool dryRun) const
    KTHROW(kul::Exception) {
  kul::hash::set::String dirs;
  for (const auto& o : objects) dirs.insert(kul::File(o).dir().real());

  std::string lib = out.dir().join(sharedLib(out.name()));
  if (mode == compiler::Mode::STAT) lib = out.dir().join(staticLib(out.name()));
  lib = kul::File(lib).esc();
  std::string cmd = linker;
  std::vector<std::string> bits;
  if (linker.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  if (mode == compiler::Mode::SHAR) p.arg("-shared").arg("-o");
  p.arg(lib);
  for (const std::string& d : dirs) p.arg(kul::File(oStar(objects), d).escm());
  {
    auto ll(kul::env::GET("MKN_LIB_LINK_LIB"));
    if ((ll.size() ? kul::String::BOOL(ll) : 0)) {
      for (const std::string& path : libPaths) p.arg("-L" + path);
      for (const std::string& lib : libs) p.arg("-l" + lib);
    }
  }
  for (const std::string& s : kul::cli::asArgs(linkerEnd)) p.arg(s);
  CompilerProcessCapture pc;
  try {
    if (!dryRun) p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(lib);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture maiken::cpp::GccCompiler::compileSource(
    const std::string& compiler, const std::vector<std::string>& args,
    const std::vector<std::string>& incs, const std::string& in,
    const std::string& out, const maiken::compiler::Mode& mode,
    bool dryRun) const KTHROW(kul::Exception) {
  std::string cmd = compiler;
  std::vector<std::string> bits;
  if (compiler.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(compiler);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
  for (const std::string& s : incs) p.arg("-I" + s);
  for (const std::string& s : args) p.arg(s);
  p.arg("-o").arg(out).arg("-c").arg(in);
  CompilerProcessCapture pc;
  if (!kul::LogMan::INSTANCE().inf()) pc.setProcess(p);
  try {
    if (!dryRun) p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

void maiken::cpp::GccCompiler::preCompileHeader(
    const std::vector<std::string>& incs, const std::vector<std::string>& args,
    const std::string& in, const std::string& out, bool dryRun) const
    KTHROW(kul::Exception) {
  if (in.rfind(".") == std::string::npos)
    KEXCEPT(Exception, "Unknown header type");

  std::string cmd;  // = compiler + " -x";
  std::string h = in.substr(in.rfind(".") + 1);

  if (h.compare("h") == 0)
    cmd = cc() + " -x c-header ";
  else if (h.compare("hpp") == 0)
    cmd = cxx() + " -x c++-header ";
  else
    KEXCEPT(Exception, "Failed to pre-compile header - uknown file type: " + h);
  cmd += in + " ";
  for (const std::string& s : args) cmd += s + " ";
  for (const std::string& s : incs) cmd += "-I" + s + " ";

  cmd += " -o " + out;
  if (kul::os::exec(cmd) != 0)
    KEXCEPT(Exception, "Failed to pre-compile header");
}