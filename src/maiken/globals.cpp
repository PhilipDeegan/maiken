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
#include "maiken/app.hpp"

namespace maiken {
std::shared_ptr<AppVars> AppVars::instance;
}

maiken::AppVars::AppVars() {
  pks["OS"] = KTOSTRING(__MKN_KUL_OS__);
  pks["HOME"] = mkn::kul::user::home().path();
  pks["MKN_HOME"] = mkn::kul::user::home(STR_MAIKEN).path();

  pks["DATETIME"] = mkn::kul::DateTime::NOW();
  pks["TIMESTAMP"] = std::time(NULL);
  auto root = Settings::INSTANCE().root();

  if (root[STR_LOCAL]) {
    auto const& local = root[STR_LOCAL];
    if (local[STR_REPO]) pks["MKN_REPO"] = mkn::kul::Dir(local[STR_REPO].Scalar()).real();
    if (local[STR_MOD_REPO]) pks["MKN_MOD_REPO"] = mkn::kul::Dir(local[STR_MOD_REPO].Scalar()).real();
  }
  if (!pks.count("MKN_REPO"))
    pks["MKN_REPO"] = mkn::kul::user::home(mkn::kul::Dir::JOIN(STR_MAIKEN, STR_REPO)).path();
  if (!pks.count("MKN_MOD_REPO"))
    pks["MKN_MOD_REPO"] = mkn::kul::user::home(mkn::kul::Dir::JOIN(STR_MAIKEN, STR_MOD_REPO)).path();

  std::string ext, pre;
#if KUL_IS_WIN
  evs["MKN_OBJ"] = "obj", ext = ".dll", pre = "";
#else
  evs["MKN_OBJ"] = "o", ext = ".so", pre = "lib";
#endif

  evs["MKN_LIB_EXT"] = ext;
  evs["MKN_LIB_PRE"] = pre;

  auto check_set = [&](std::string const key) {
    auto cstr = key.c_str();
    if (mkn::kul::env::EXISTS(cstr)) evs[key] = mkn::kul::env::GET(cstr);
  };
  check_set("MKN_OBJ");
  check_set("MKN_LIB_EXT");
  check_set("MKN_LIB_PRE");
}
