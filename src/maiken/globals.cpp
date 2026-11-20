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
#include "mkn/kul/env.hpp"
#include "maiken/global.hpp"
#include "maiken/settings.hpp"
#include "mkn/kul/defs.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/time.hpp"

#include <ctime>
#include <memory>
#include <optional>
#include <string>

namespace maiken {
std::shared_ptr<AppVars> AppVars::instance;
}

maiken::AppVars::AppVars() {
  pks["OS"] = MKN_KUL_STR(MKN_KUL_OS);
  pks["HOME"] = mkn::kul::user::home().string();
  pks["ARCH"] = mkn::kul::env::GET("MKN_ARCH", "x86_64");
  pks["MKN_HOME"] = mkn::kul::user::home(STR_MAIKEN).string();

  pks["DATETIME"] = mkn::kul::DateTime::NOW();
  pks["TIMESTAMP"] = std::time(NULL);

  std::string ext, pre;
#if MKN_KUL_IS_WIN
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

void maiken::AppVars::POST_CONSTRUCT() {
  auto& pks = AppVars::INSTANCE().pks;

  auto const& settings = Settings::INSTANCE();
  if (auto const& local_repo = settings.local_dep_repo())
    pks["MKN_REPO"] = *local_repo;
  else
    pks["MKN_REPO"] = (mkn::kul::user::home(STR_MAIKEN) / STR_REPO).string();

  if (auto const& local_repo = settings.local_mod_repo())
    pks["MKN_MOD_REPO"] = *local_repo;
  else
    pks["MKN_MOD_REPO"] = (mkn::kul::user::home(STR_MAIKEN) / STR_MOD_REPO).string();

  // prebuilt binaries are matched against mkn.bin.key and used for every dependency with
  //  mkn.with.bin, see prebuilt.cpp; settings' property: can set them, MKN_WITH_BIN overrides
  //  mkn.with.bin (e.g. one env: line in a CI workflow), -P overrides both
  pks["mkn.bin.key"] = defaultBinKey();
  pks["mkn.with.bin"] = "0";
  for (auto const* const k : {"mkn.bin.key", "mkn.with.bin"})
    for (auto const* s = &settings; s != nullptr; s = s->super())
      if (auto const ps = s->root()[STR_PROPERTY]; ps && ps[k] && ps[k].IsScalar()) {
        pks[k] = ps[k].Scalar();
        break;
      }
  if (mkn::kul::env::EXISTS("MKN_WITH_BIN"))
    pks["mkn.with.bin"] = mkn::kul::env::GET("MKN_WITH_BIN");
}
