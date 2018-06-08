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
#include "kul/log.hpp"

std::vector<std::string> maiken::Regexer::RESOLVE_REGEX(std::string str)
    KTHROW(kul::Exception) {
  std::vector<std::string> v;
  auto posL = str.find("(");
  auto posR = str.find(")");

  if (posL == std::string::npos || posR == std::string::npos) return v;
  if (str.size() > 1 && str.substr(0, 2) == "./") str = str.substr(2);

  kul::Dir d(str);
  std::string built, prnt;
  std::vector<std::string> bits;
  bits.insert(bits.begin(), d.name());
  while (d.parent().name() != prnt && !d.parent().root()) {
    bits.insert(bits.begin(), d.parent().name());
    prnt = d.parent().name();
    d = d.parent();
  }
  if (d.parent().root()) {
    bits.insert(bits.begin(), d.parent().name());
  } else {
    d = kul::env::CWD();
    bits.insert(bits.begin(), d.name());
    while (d.parent().name() != prnt && !d.parent().root()) {
      bits.insert(bits.begin(), d.parent().name());
      prnt = d.parent().name();
      d = d.parent();
    }
    str = kul::env::CWD() + kul::Dir::SEP() + str;
  }

  std::string rem, rule;
  size_t bitsIndex = 0;
  for (const auto& s : bits) {
    auto posL = s.find("(");
    auto posR = s.find(")");
    if (posL != std::string::npos && posR != std::string::npos) {
      if (built.size() + s.size() + 2 > str.size()) {
        rem = str.substr(built.size() + s.size() + 1);
      } else
        rem = str.substr(built.size() + s.size() + 2);
      rule = s;
      break;
    }
    if (kul::Dir(s).root())
      built = s;
    else if (kul::Dir(built).root())
      built = built + s;
    else
      built = built + kul::Dir::SEP() + s;
    bitsIndex++;
  }
  d = built;

  auto regexer = [&](auto items) {
    for (const auto& item : items) {
      try {
        std::regex re(str);
        std::smatch match;
        std::string subject(item.real());
        if (std::regex_search(subject, match, re) && match.size() > 1)
          RESOLVE_REGEX_REC(item.real(), built, subject, rem, bits, bitsIndex,
                            v);
      } catch (std::regex_error& e) {
        KEXIT(1, "Regex Failure:\n") << e.what();
      }
    }
  };
  regexer(d.files(1));
  return v;
}

void maiken::Regexer::RESOLVE_REGEX_REC(
    const std::string& i, const std::string& b, const std::string& s,
    const std::string& r, const std::vector<std::string>& bits,
    const size_t& bitsIndex, std::vector<std::string>& v)
    KTHROW(kul::Exception) {
  if (kul::File(i).is() && !kul::Dir(i).is()) {
    v.push_back(i);
    return;
  }
}
