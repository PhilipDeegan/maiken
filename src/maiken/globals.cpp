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
#include "maiken/global.hpp"

namespace maiken {
std::shared_ptr<AppVars> AppVars::instance;
}

maiken::AppVars::AppVars() {
  pks["OS"] = KTOSTRING(__KUL_OS__);
  pks["HOME"] = kul::user::home().path();
  pks["MKN_HOME"] = kul::user::home(STR_MAIKEN).path();
  pks["DATETIME"] = kul::DateTime::NOW();
  pks["TIMESTAMP"] = std::time(NULL);

  if (Settings::INSTANCE().root()[STR_LOCAL] &&
      Settings::INSTANCE().root()[STR_LOCAL][STR_REPO])
    pks["MKN_REPO"] = Settings::INSTANCE().root()[STR_LOCAL][STR_REPO].Scalar();
  else
    pks["MKN_REPO"] =
        kul::user::home(kul::Dir::JOIN(STR_MAIKEN, STR_REPO)).path();

  if (Settings::INSTANCE().root()[STR_LOCAL] &&
      Settings::INSTANCE().root()[STR_LOCAL][STR_MOD_REPO])
    pks["MKN_MOD_REPO"] =
        Settings::INSTANCE().root()[STR_LOCAL][STR_MOD_REPO].Scalar();
  else
    pks["MKN_MOD_REPO"] =
        kul::user::home(kul::Dir::JOIN(STR_MAIKEN, STR_MOD_REPO)).path();

  if (Settings::INSTANCE().root()[STR_LOCAL] &&
      Settings::INSTANCE().root()[STR_LOCAL][STR_BIN])
    pks["MKN_BIN"] = Settings::INSTANCE().root()[STR_LOCAL][STR_BIN].Scalar();
  if (Settings::INSTANCE().root()[STR_LOCAL] &&
      Settings::INSTANCE().root()[STR_LOCAL][STR_LIB])
    pks["MKN_LIB"] = Settings::INSTANCE().root()[STR_LOCAL][STR_LIB].Scalar();

  evs["MKN_OBJ"] = "o";
  std::string obj = kul::env::GET("MKN_OBJ");
  if (!obj.empty()) evs["MKN_OBJ"] = obj;

#ifdef _WIN32
  evs["MKN_LIB_EXT"] = "dll";
  evs["MKN_LIB_PRE"] = "";
#else
  evs["MKN_LIB_EXT"] = "so";
  evs["MKN_LIB_PRE"] = "lib";
#endif
  std::string ext = kul::env::GET("MKN_LIB_EXT");
  if (!ext.empty()) evs["MKN_LIB_EXT"] = ext;

  std::string pre = kul::env::GET("MKN_LIB_PRE");
  if (!pre.empty()) evs["MKN_LIB_PRE"] = pre;
}
