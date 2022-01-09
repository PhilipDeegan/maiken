/**
Copyright (c) 2022, Philip Deegan.
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
#include "maiken/module/init.hpp"

namespace maiken::test {

class MaikenModule : public maiken::Module {
 public:
  void init(Application&, YAML::Node const&) KTHROW(std::exception) override { init_ = 1; }
  void compile(Application&, YAML::Node const&) KTHROW(std::exception) override { compile_ = 1; }
  void link(Application&, YAML::Node const&) KTHROW(std::exception) override { link_ = 1; }
  void test(Application&, YAML::Node const&) KTHROW(std::exception) override { test_ = 1; }
  void pack(Application&, YAML::Node const&) KTHROW(std::exception) override { pack_ = 1; }

  ~MaikenModule() {
    auto const vec = {
        init_, compile_, link_, test_, pack_,
    };
    if (!std::all_of(vec.begin(), vec.end(), [](auto& v) { return v == 1; })) std::abort();
  };

 private:
  bool init_ = 0;
  bool compile_ = 0;
  bool link_ = 0;
  bool test_ = 0;
  bool pack_ = 0;
};

}  // namespace maiken::test

extern "C" KUL_PUBLISH maiken::Module* maiken_module_construct() {
  return new maiken ::test ::MaikenModule;
}

extern "C" KUL_PUBLISH void maiken_module_destruct(maiken::Module* p) { delete p; }
