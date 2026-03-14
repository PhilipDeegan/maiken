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
#include "maiken/app.hpp"
#include "maiken/property.hpp"

#include <optional>

void maiken::Application::findables() {
  if (!this->ro) return;  // skip otherwise
  if (lang.empty()) {
    KLOG(ERR) << "Unknown language: cannot interrogate compiler for automatic file finding/etc";
    return;
  }

  auto try_find_file = [&](auto const& k, [[maybe_unused]] auto const& l,
                           std::vector<std::string> const ts,
                           auto const& lines) -> std::optional<mkn::kul::File> {
    for (auto const& line : lines) {
      auto const resolved = Properties::RESOLVE(*this, line);
      for (auto const& type : ts)
        if (auto file = mkn::kul::File(k + type, resolved); file.is()) return file;
    }

    return std::nullopt;
  };

  auto const incs = [&](auto const& k, auto const& l, auto const& lines) {
    if (auto const file = try_find_file(k, l, {".h", ".hpp"}, lines))
      this->addInclude(file->dir().real());
  };
  auto const libs = [&](auto const& k, auto const& l, auto const& lines) {
    auto const prefix = AppVars::INSTANCE().envVar("MKN_LIB_PRE");
    auto const type = AppVars::INSTANCE().envVar("MKN_LIB_EXT");
    if (auto const file = try_find_file(prefix + k, prefix + l, {type}, lines))
      this->addLibpath(file->dir().real());
  };

  auto const& findables = Settings::INSTANCE().findables();

  for (auto const& [k, v] : findables)
    for (auto const& l : libraries()) {
      if (l.starts_with(k)) {
        if (v.count("inc")) incs(k, l, v["inc"]);
        if (v.count("lib")) libs(k, l, v["lib"]);
      }
    }
}
