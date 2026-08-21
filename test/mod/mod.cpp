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
#include "mkn/mod/init.hpp"

#include "dep.hpp"

namespace maiken::test {

class MaikenModule : public mkn::mod::Module {
 public:
  void init(mkn::mod::Context&, YAML::Node const&) override {
    init_ = 1;
    // test_mod_dep itself links parse.yaml as a shared library, so loading
    // this module transitively needs libparse.yaml resolvable too - not just
    // libtest_mod_dep.so.
    if (test_mod_dep_value() != 1234 || !test_mod_dep_yaml_ok()) std::abort();
  }
  void compile(mkn::mod::Context&, YAML::Node const& node) override {
    // Every real module signature carries a YAML::Node, and real modules read
    // it with operator[]/as<T>(), which throws a YAML::Exception (vtable/RTTI
    // defined in libparse.yaml, not this module's own object code) on a
    // missing/mistyped key - exercising that here forces this .so to actually
    // depend on that symbol at load time, same as a real module.
    try {
      node["mkn_test_mod_missing_key"].as<std::string>();
      std::abort();
    } catch (YAML::Exception const&) {
    }
    compile_ = 1;
  }
  void link(mkn::mod::Context&, YAML::Node const&) override { link_ = 1; }
  void test(mkn::mod::Context&, YAML::Node const&) override { test_ = 1; }
  void pack(mkn::mod::Context&, YAML::Node const&) override { pack_ = 1; }

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

extern "C" MKN_KUL_PUBLISH mkn::mod::Module* maiken_module_construct() {
  return new maiken::test::MaikenModule;
}

extern "C" MKN_KUL_PUBLISH void maiken_module_destruct(mkn::mod::Module* p) { delete p; }
