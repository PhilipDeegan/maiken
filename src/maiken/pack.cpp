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
#include "kul/string.hpp"
#include "maiken.hpp"

class LibFinder {
public:
  static bool findAdd(const std::string &l, const kul::Dir &i,
                      const kul::Dir &o) {
    bool found = 0;
    for (const auto &f : i.files(0)) {
      const auto &fn(f.name());
      if (fn.find(".") == std::string::npos)
        continue;
#ifdef _WIN32
      if (fn.substr(0, fn.rfind(".")) == l) {
#else
      if (fn.size() > (3 + l.size()) && fn.substr(0, 3) == "lib" &&
          kul::String::NO_CASE_CMP(fn.substr(3, l.size()), l)) {
        auto bits(kul::String::SPLIT(fn.substr(3 + l.size()), '.'));
#ifdef __APPLE__
        if (bits[bits.size() - 1] != "dyn"
#else
        if (!(bits[0] == "so" || bits[bits.size() - 1] == "so")
#endif //__APPLE__
            && bits[bits.size() - 1] != "a")
          continue;

#endif //_WIN32
        f.cp(o);
        found = 1;
      }
    }
    return found;
  }
};

void maiken::Application::pack() KTHROW(kul::Exception) {
  kul::Dir pk(buildDir().join("pack"));
  if (!pk && !pk.mk())
    KEXIT(1, "Cannot create: " + pk.path());

  kul::Dir bin(pk.join("bin"), main.size());
  kul::Dir lib(pk.join("lib"));

  if (!main.empty() || !srcs.empty()) {
    const auto v((inst ? inst : buildDir()).files(0));
    if (v.empty())
      KEXIT(1, "Current project lib/bin not found during pack");
    for (const auto &f : v)
      f.cp(main.size() ? bin : lib);
  }

  for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app)
    if (!(*app)->srcs.empty()) {
      auto &a = **app;
      kul::Dir outD(a.inst ? a.inst.real() : a.buildDir());
      std::string n = a.project().root()[STR_NAME].Scalar();
      if (!LibFinder::findAdd(a.baseLibFilename(), outD, lib))
        KEXIT(1, "Depedency Project lib not found, try building: ")
            << a.buildDir().real();
    }
  for (const auto &l : libs) {
    bool found = 0;
    for (const auto &p : paths) {
      kul::Dir path(p);
      if (!path)
        KEXIT(1, "Path does not exist: ") << pk.path();
      found = LibFinder::findAdd(l, path, lib);
      if (found)
        break;
    }
    if (!found)
      KOUT(NON) << "WARNING - Library not found during pack: " << l;
  }
}