/**
Copyright (c) 2018, Philip Deegan.
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
#include "maiken/regex.hpp"
#include <regex>
#include "mkn/kul/log.hpp"

std::vector<std::string> maiken::Regexer::RESOLVE(std::string str) KTHROW(mkn::kul::Exception) {
  std::vector<std::string> v;
  auto posL = str.find("(");
  auto posR = str.find(")");

  if (posL == std::string::npos || posR == std::string::npos) return v;
  if (str.size() > 1 && str.substr(0, 2) == "./") str = str.substr(2);

  auto bits = mkn::kul::String::SPLIT(str, "/");
  str = bits[bits.size() - 1];
  mkn::kul::Dir d(mkn::kul::env::CWD());

  if (bits.size() > 1) d = mkn::kul::Dir(bits[0]);
  for (size_t i = 1; i < bits.size() - 1; i++) d = d.join(bits[i]);

  auto regexer = [&](auto items) {
    for (auto const& item : items) {
      try {
        std::regex re(str);
        std::smatch match;
        std::string subject(item.real());
        if (std::regex_search(subject, match, re) && match.size() > 1) RESOLVE_REC(item.real(), v);
      } catch (std::regex_error& e) {
        KEXIT(1, "Regex Failure:\n") << e.what();
      }
    }
  };
  regexer(d.files(1));
  return v;
}

void maiken::Regexer::RESOLVE_REC(std::string const& i, std::vector<std::string>& v)
    KTHROW(mkn::kul::Exception) {
  if (mkn::kul::File(i).is() && !mkn::kul::Dir(i).is()) {
    v.push_back(i);
    return;
  }
}
