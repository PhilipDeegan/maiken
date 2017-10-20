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

maiken::CompilerProcessCapture
maiken::cpp::GccCompiler::buildExecutable(
  const std::string& linker,
  const std::string& linkerEnd,
  const std::vector<std::string>& objects,
  const std::vector<std::string>& libs,
  const std::vector<std::string>& libPaths,
  const std::string& out,
  const maiken::compiler::Mode& mode,
  bool dryRun) const KTHROW(kul::Exception)
{
  kul::hash::set::String dirs;
  for (const auto& o : objects)
    dirs.insert(kul::File(o).dir().real());

  std::string cmd = linker;
  std::vector<std::string> bits;
  if (linker.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++)
    p.arg(bits[i]);
  for (const std::string& path : libPaths)
    p.arg("-L" + path);
  if (mode == compiler::Mode::STAT)
    p.arg("-static");
  p.arg("-o").arg(out);
  for (const std::string& d : dirs)
    p.arg(kul::File(oStar(objects), d).escm());
  for (const std::string& lib : libs)
    p.arg("-l" + lib);
  for (const std::string& s : kul::cli::asArgs(linkerEnd))
    p.arg(s);

  CompilerProcessCapture pc;
  try {
    if (!dryRun)
      p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture
maiken::cpp::GccCompiler::buildLibrary(const std::string& linker,
                                       const std::string& linkerEnd,
                                       const std::vector<std::string>& objects,
                                       const std::vector<std::string>& libs,
                                       const std::vector<std::string>& libPaths,
                                       const kul::File& out,
                                       const maiken::compiler::Mode& mode,
                                       bool dryRun) const KTHROW(kul::Exception)
{
  kul::hash::set::String dirs;
  for (const auto& o : objects)
    dirs.insert(kul::File(o).dir().real());

  std::string lib = out.dir().join(sharedLib(out.name()));
  if (mode == compiler::Mode::STAT)
    lib = out.dir().join(staticLib(out.name()));
  lib = kul::File(lib).esc();
  std::string cmd = linker;
  std::vector<std::string> bits;
  if (linker.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(linker);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++)
    p.arg(bits[i]);
  if (mode == compiler::Mode::SHAR)
    p.arg("-shared").arg("-o");
  p.arg(lib);
  for (const std::string& d : dirs)
    p.arg(kul::File(oStar(objects), d).escm());
  {
    auto ll(kul::env::GET("MKN_LIB_LINK_LIB"));
    if ((ll.size() ? kul::String::BOOL(ll) : 0)) {
      for (const std::string& path : libPaths)
        p.arg("-L" + path);
      for (const std::string& lib : libs)
        p.arg("-l" + lib);
    }
  }
  for (const std::string& s : kul::cli::asArgs(linkerEnd))
    p.arg(s);
  CompilerProcessCapture pc;
  try {
    if (!dryRun)
      p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(lib);
  pc.cmd(p.toString());
  return pc;
}

maiken::CompilerProcessCapture
maiken::cpp::GccCompiler::compileSource(const std::string& compiler,
                                        const std::vector<std::string>& args,
                                        const std::vector<std::string>& incs,
                                        const std::string& in,
                                        const std::string& out,
                                        const maiken::compiler::Mode& mode,
                                        bool dryRun) const
  KTHROW(kul::Exception)
{
  std::string cmd = compiler;
  std::vector<std::string> bits;
  if (compiler.find(" ") != std::string::npos) {
    bits = kul::cli::asArgs(compiler);
    cmd = bits[0];
  }
  kul::Process p(cmd);
  for (unsigned int i = 1; i < bits.size(); i++)
    p.arg(bits[i]);
  for (const std::string& s : incs)
    p.arg("-I" + s);
  for (const std::string& s : args)
    p.arg(s);
  p.arg("-o").arg(out).arg("-c").arg(in);
  CompilerProcessCapture pc;
  if (!kul::LogMan::INSTANCE().inf())
    pc.setProcess(p);
  try {
    if (!dryRun)
      p.start();
  } catch (const kul::proc::Exception& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  pc.cmd(p.toString());
  return pc;
}

void
maiken::cpp::GccCompiler::preCompileHeader(const std::vector<std::string>& incs,
                                           const std::vector<std::string>& args,
                                           const std::string& in,
                                           const std::string& out,
                                           bool dryRun) const
  KTHROW(kul::Exception)
{
  if (in.rfind(".") == std::string::npos)
    KEXCEPT(Exception, "Unknown header type");

  std::string cmd; // = compiler + " -x";
  std::string h = in.substr(in.rfind(".") + 1);

  if (h.compare("h") == 0)
    cmd = cc() + " -x c-header ";
  else if (h.compare("hpp") == 0)
    cmd = cxx() + " -x c++-header ";
  else
    KEXCEPT(Exception, "Failed to pre-compile header - uknown file type: " + h);
  cmd += in + " ";
  for (const std::string& s : args)
    cmd += s + " ";
  for (const std::string& s : incs)
    cmd += "-I" + s + " ";

  cmd += " -o " + out;
  if (kul::os::exec(cmd) != 0)
    KEXCEPT(Exception, "Failed to pre-compile header");
}