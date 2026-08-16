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

// Prototype only - not wired into maiken/compiler.hpp yet. Sketches the
// CommandDAO/ArgGenerator shape proposed for the Compiler hierarchy so it can
// be reviewed before touching CompileDAO/LinkDAO/ThreadingCompiler for real.

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace proto {

// Pull-based: advances and writes the next token into out, or returns false
// once exhausted. No stage ever holds the full, concatenated arg list.
using ArgGenerator = std::function<bool(std::string&)>;

ArgGenerator fromVector(std::shared_ptr<std::vector<std::string> const> const& v) {
  auto idx = std::make_shared<size_t>(0);
  return [idx, v](std::string& out) {
    if (*idx >= v->size()) return false;
    out = (*v)[(*idx)++];
    return true;
  };
}

ArgGenerator single(std::string s) {
  auto done = std::make_shared<bool>(false);
  return [done, s = std::move(s)](std::string& out) {
    if (*done) return false;
    out = s;
    *done = true;
    return true;
  };
}

// Exhausts each source generator in order before advancing to the next -
// how per-filetype args, cArg, debug/opt/warn flags, and per-source args
// would compose without ever being copied into one combined vector first.
ArgGenerator chain(std::vector<ArgGenerator> sources) {
  auto idx = std::make_shared<size_t>(0);
  auto gens = std::make_shared<std::vector<ArgGenerator>>(std::move(sources));
  return [idx, gens](std::string& out) {
    while (*idx < gens->size()) {
      if ((*gens)[*idx](out)) return true;
      ++*idx;
    }
    return false;
  };
}

struct CommandDAO {
  std::string prefix;
  std::string binary;
  ArgGenerator args;  // single-pass: drain once, then discard
};

std::vector<std::string> drain(ArgGenerator gen) {
  std::vector<std::string> out;
  std::string tok;
  while (gen(tok)) out.push_back(tok);
  return out;
}

std::string toString(CommandDAO const& cmd) {
  std::string s;
  if (!cmd.prefix.empty()) s += cmd.prefix + " ";
  s += cmd.binary;
  auto gen = cmd.args;
  std::string tok;
  while (gen(tok)) s += " " + tok;
  return s;
}

}  // namespace proto

namespace {

int failures = 0;

void expect_eq(std::string const& what, std::string const& got, std::string const& want) {
  if (got != want) {
    std::cerr << "FAIL " << what << ": got \"" << got << "\", want \"" << want << "\"\n";
    ++failures;
  }
}

void expect_eq(std::string const& what, std::vector<std::string> const& got,
               std::vector<std::string> const& want) {
  if (got != want) {
    std::cerr << "FAIL " << what << ": got " << got.size() << " token(s), want " << want.size()
              << "\n";
    ++failures;
  }
}

}  // namespace

int main() {
  using namespace proto;

  // Sources stay as independently-owned small vectors/singles until drained -
  // "defines", "std flag", and "per-source args" are never concatenated
  // upfront the way ThreadingCompiler::compilationUnit's `args` vector is today.
  auto defines = std::make_shared<std::vector<std::string>>(
      std::vector<std::string>{"-DFOO", "-DBAR"});
  auto perSource = std::make_shared<std::vector<std::string>>(
      std::vector<std::string>{"-Wall", "-Wextra"});

  CommandDAO cmd{"ccache", "g++-10",
                 chain({fromVector(defines), single("-std=c++20"), fromVector(perSource)})};

  expect_eq("drain(cmd.args)", drain(cmd.args), {"-DFOO", "-DBAR", "-std=c++20", "-Wall", "-Wextra"});

  CommandDAO cmd2{"ccache", "g++-10",
                  chain({fromVector(defines), single("-std=c++20"), fromVector(perSource)})};
  expect_eq("toString(cmd2)", toString(cmd2),
            "ccache g++-10 -DFOO -DBAR -std=c++20 -Wall -Wextra");

  // A no-prefix binary omits the leading token entirely rather than emitting
  // an empty one - mirrors identify().prefix being empty for a bare "g++".
  CommandDAO cmd3{"", "g++", chain({single("-c"), single("a.cpp")})};
  expect_eq("toString(cmd3, no prefix)", toString(cmd3), "g++ -c a.cpp");

  // A generator is single-pass: draining it twice is a caller bug, not
  // something this layer silently protects against (matches the shared_ptr
  // index being mutated in place, not copied per-call).
  CommandDAO cmd4{"", "g++", fromVector(defines)};
  auto first = drain(cmd4.args);
  auto second = drain(cmd4.args);
  expect_eq("drain(cmd4.args) first pass", first, {"-DFOO", "-DBAR"});
  expect_eq("drain(cmd4.args) second pass is empty (already consumed)", second, {});

  if (failures) {
    std::cerr << failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "OK\n";
  return 0;
}
