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
#include "maiken/compiler/compilers.hpp"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect_eq(std::string const& what, std::string const& got, std::string const& want) {
  if (got != want) {
    std::cerr << "FAIL " << what << ": got \"" << got << "\", want \"" << want << "\"\n";
    ++failures;
  }
}

void expect_true(std::string const& what, bool cond) {
  if (!cond) {
    std::cerr << "FAIL " << what << "\n";
    ++failures;
  }
}

}  // namespace

int main() {
  using maiken::Compilers;

  auto& compilers = Compilers::INSTANCE();

  // Plain, already-registered names still resolve exactly as before.
  expect_eq("base(g++)", compilers.base("g++"), "g++");
  expect_eq("base(gcc)", compilers.base("gcc"), "gcc");

  // Version-suffixed binaries resolve without any explicit mask.
  expect_eq("base(g++-10)", compilers.base("g++-10"), "g++");
  expect_eq("base(clang++-17)", compilers.base("clang++-17"), "clang++");
  expect_eq("base(gcc-13.2)", compilers.base("gcc-13.2"), "gcc");

  // Target-triple prefixed + versioned binaries resolve too.
  expect_eq("base(x86_64-linux-gnu-g++-12)", compilers.base("x86_64-linux-gnu-g++-12"), "g++");
  expect_eq("base(arm-none-eabi-gcc)", compilers.base("arm-none-eabi-gcc"), "gcc");

  // A compiler string wrapped in ccache with baked-in flags: family resolves
  // to the canonical "g++" even though the string never contains that token
  // in isolation, and get() dispatches to the same GccCompiler as plain g++.
  {
    std::string const raw = "ccache g++-10 -std=c++17 -fPIC";
    expect_eq("base(ccache g++-10 ...)", compilers.base(raw), "g++");
    expect_true("get(ccache g++-10 ...) == get(g++)",
                compilers.get(raw) == compilers.get("g++"));

    auto const id = compilers.identify(raw);
    expect_eq("identify(...).family", id.family, "g++");
    expect_eq("identify(...).binary", id.binary, "g++-10");
    expect_true("identify(...).prefix == [ccache]",
                id.prefix.size() == 1 && id.prefix[0] == "ccache");
    expect_true("identify(...).trailing == [-std=c++17, -fPIC]",
                id.trailing.size() == 2 && id.trailing[0] == "-std=c++17" &&
                    id.trailing[1] == "-fPIC");
  }

  // Archiver-style strings: the binary's own leading flag is not mistaken
  // for a wrapper prefix (no family is expected to match "ar").
  {
    auto const id = compilers.identify("ar -cr");
    expect_eq("identify(ar -cr).binary", id.binary, "ar");
    expect_true("identify(ar -cr).prefix empty", id.prefix.empty());
    expect_true("identify(ar -cr).trailing == [-cr]",
                id.trailing.size() == 1 && id.trailing[0] == "-cr");
    expect_true("identify(ar -cr).family empty", id.family.empty());
  }

  // A single unwrapped, unversioned binary has no prefix/trailing at all.
  {
    auto const id = compilers.identify("g++");
    expect_true("identify(g++).prefix empty", id.prefix.empty());
    expect_eq("identify(g++).binary", id.binary, "g++");
    expect_true("identify(g++).trailing empty", id.trailing.empty());
    expect_eq("identify(g++).family", id.family, "g++");
  }

  // Explicit masks (for names the automatic rules can't infer) still work,
  // and base() now correctly reports their canonical family (previously a
  // dead try/catch meant base() never consulted masks at all).
  {
    compilers.addMask("mycustomcc", "g++");
    expect_eq("base(mycustomcc)", compilers.base("mycustomcc"), "g++");
    expect_true("get(mycustomcc) == get(g++)",
                compilers.get("mycustomcc") == compilers.get("g++"));
  }

  if (failures) {
    std::cerr << failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "OK\n";
  return 0;
}
