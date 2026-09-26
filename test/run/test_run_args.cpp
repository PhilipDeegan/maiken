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

// Builds test/run/args with the freshly built mkn, then checks run arguments
// given via -r and after -- reach the binary, -r first when both are used.

#include "mkn/kul/proc.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

std::string mkn_bin;

std::string const PROJECT = "test/run/args";

std::string capture(std::vector<std::string> const& args) {
  mkn::kul::Process p(mkn_bin);
  // args are parsed shell style, spaces must be quoted or backslashed as on a command line
  for (auto const& a : args) p.arg(a.find(' ') == std::string::npos ? a : "\"" + a + "\"");
  mkn::kul::ProcessCapture pc(p);
  try {
    p.start();
  } catch (std::exception const& e) {
    std::cerr << "FAIL mkn " << args[0] << ": " << e.what() << "\n" << pc.errs() << "\n";
    ++failures;
  }
  return pc.outs();
}

std::string args_line(std::string const& out) {
  std::istringstream ss(out);
  for (std::string line; std::getline(ss, line);)
    if (line.rfind("ARGS:", 0) == 0) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      return line;
    }
  return "";
}

void expect_run(std::string const& what, std::vector<std::string> const& extra,
                std::string const& want) {
  std::vector<std::string> args{"run", "-C", PROJECT};
  args.insert(args.end(), extra.begin(), extra.end());
  auto const got = args_line(capture(args));
  if (got != want) {
    std::cerr << "FAIL " << what << ": got \"" << got << "\", want \"" << want << "\"\n";
    ++failures;
  }
}

}  // namespace

int main(int /*argc*/, char* argv[]) {
  // tests are built to bin/<profile>/test/, mkn is linked to bin/<profile>/
  auto const bin_dir = std::filesystem::absolute(argv[0]).parent_path().parent_path();
#ifdef _WIN32
  mkn_bin = (bin_dir / "mkn.exe").string();
#else
  mkn_bin = (bin_dir / "mkn").string();
#endif

  capture({"build", "-C", PROJECT, "-q"});
  if (failures) return failures;

  expect_run("no args", {}, "ARGS:");
  expect_run("-r", {"-r", "a0 a1"}, "ARGS:[a0][a1]");
  expect_run("--", {"--", "b0", "b1"}, "ARGS:[b0][b1]");
  expect_run("-r and --", {"-r", "a0 a1", "--", "b0", "b1"}, "ARGS:[a0][a1][b0][b1]");

  if (failures) std::cerr << failures << " failure(s)\n";
  return failures;
}
