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

#include "maiken/settings.hpp"

void maiken::Settings::write(mkn::kul::File const& file) KTHROW(mkn::kul::Exit) {
  mkn::kul::io::Writer w(file);
  w.write("\n", true);

  bool c = mkn::kul::env::WHICH("clang") || mkn::kul::env::WHICH("clang.exe");
  bool g = mkn::kul::env::WHICH("gcc") || mkn::kul::env::WHICH("gcc.exe");
  {
    auto gcc_pref(mkn::kul::env::GET("MKN_GCC_PREFERRED"));
    bool use_gcc = gcc_pref.empty() ? 0 : mkn::kul::String::BOOL(gcc_pref);
    if (c && g && use_gcc) c = 0;
  }

  w.write("#local:", true);
  w.write("## Optionnaly override local repository directory", true);
  w.write("#  repo: <directory>", true);
  w << mkn::kul::os::EOL();

#ifdef _WIN32
  auto cl_pref(mkn::kul::env::GET("MKN_CL_PREFERRED"));
  bool use_cl = cl_pref.empty() ? 0 : mkn::kul::String::BOOL(cl_pref);
  if (!use_cl && (c || g)) {
#else
  if (c || g) {
#endif
    w.write("## Add include directories to every compilation", true);
    w.write("#inc: <directory>", true);
    w.write("## Add library paths when linking every binary", true);
    w.write("#path:    <directory>\n", true);
    w << mkn::kul::os::EOL();

    w.write(
        "## Modify environement variables for application commands - excludes "
        "run",
        true);
    w.write("#env:", true);
    w.write("#- name: VAR", true);
    w.write("#  mode: prepend", true);
    w.write("#  value: value", true);
    w << mkn::kul::os::EOL();

    w.write("file:", true);
    w.write("- type: c:S", true);
    w.write("  archiver: ar -cr", true);
    w << "  compiler: " << (c ? "clang" : "gcc") << mkn::kul::os::EOL();
    w << "  linker: " << (c ? "clang" : "gcc") << mkn::kul::os::EOL();
    w.write("- type: cpp:cxx:cc", true);
    w.write("  archiver: ar -cr", true);
    w << "  compiler: " << (c ? "clan" : "") << "g++" << mkn::kul::os::EOL();
    w << "  linker: " << (c ? "clan" : "") << "g++" << mkn::kul::os::EOL();
    w << mkn::kul::os::EOL();
  }

#ifdef _WIN32

  if (use_cl || (!c && !g)) {
    auto cl(mkn::kul::env::WHERE("cl.exe"));
    auto inc(mkn::kul::env::GET("INCLUDE"));
    auto lib(mkn::kul::env::GET("LIB"));

    if (cl.empty() || inc.empty() || lib.empty()) {
      w.flush().close();
      file.rm();
      KEXIT(1, "gcc or clang not found, vcvars not detected")
          << mkn::kul::os::EOL()
          << "\tRun vcvarsall.bat or view mkn wiki to see how to configure "
             "maiken settings.yaml"
          << mkn::kul::os::EOL() << "\t@ https://github.com/mkn/mkn/wiki";
    }

    w.write("inc: ", true);
    for (auto s : mkn::kul::String::SPLIT(inc, mkn::kul::env::SEP())) {
      mkn::kul::String::REPLACE_ALL(s, "\\", "/");
      mkn::kul::String::REPLACE_ALL(s, " ", "\\ ");
      w << "  " << s << mkn::kul::os::EOL();
    }
    w << mkn::kul::os::EOL();

    w.write("path: ", true);
    for (auto s : mkn::kul::String::SPLIT(lib, mkn::kul::env::SEP())) {
      mkn::kul::String::REPLACE_ALL(s, "\\", "/");
      mkn::kul::String::REPLACE_ALL(s, " ", "\\ ");
      w << "  " << s << mkn::kul::os::EOL();
    }
    w << mkn::kul::os::EOL();

    w.write("env:", true);
    w.write("- name: PATH", true);
    w.write("  mode: prepend", true);
    w << "  value: " << mkn::kul::File(cl).dir().real() << mkn::kul::os::EOL();
    w << mkn::kul::os::EOL();

    w.write("file:", true);
    w.write("- type: cpp:cxx:cc:c", true);
    w.write("  archiver: lib", true);
    w.write("  compiler: cl", true);
    w.write("  linker: link", true);
  }
#endif
  w << mkn::kul::os::EOL();

  w.write("# Other examples", true);
  w.write("#- type: cu", true);
  w.write("#  archiver: ar -cr", true);
  w.write("#  compiler: nvcc", true);
  w.write("#  linker: nvcc", true);

  w.write("#- type: m", true);
  w.write("#  archiver: ar -cr", true);
  w.write("#  compiler: g++ -lobjc", true);
  w.write("#  linker: g++", true);

  w.write("#- type: cs", true);
  w.write("#  compiler: csc", true);
  w.write("#  linker: csc", true);
}
