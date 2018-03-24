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
#ifndef _MAIKEN_GLOBALS_HPP_
#define _MAIKEN_GLOBALS_HPP_

#include "kul/map.hpp"

namespace maiken {

class AppVars : public Constants
{
private:
  bool dr = 0, f = 0, q = 0, s = 0, sh = 0, st = 0, u = 0;
  uint16_t de = -1, dl = 0, op = -1, ts = 1, wa = -1;
  std::string aa, al, la, ra, wi, wo;
  const std::string* dep;
  kul::hash::set::String cmds, modPhases, wop;
  kul::hash::map::S2S evs, jas, pks;
  AppVars()
  {
    pks["OS"] = KTOSTRING(__KUL_OS__);
    pks["HOME"] = kul::user::home().path();
    pks["MKN_HOME"] = kul::user::home(STR_MAIKEN).path();
    pks["DATETIME"] = kul::DateTime::NOW();
    pks["TIMESTAMP"] = std::time(NULL);

    if (Settings::INSTANCE().root()[STR_LOCAL] &&
        Settings::INSTANCE().root()[STR_LOCAL][STR_REPO])
      pks["MKN_REPO"] =
        Settings::INSTANCE().root()[STR_LOCAL][STR_REPO].Scalar();
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
    if (!obj.empty())
      evs["MKN_OBJ"] = obj;

#ifdef _WIN32
    evs["MKN_LIB_EXT"] = "dll";
#elif  _APPLE_
    evs["MKN_LIB_EXT"] = "dylib";
#else
    evs["MKN_LIB_EXT"] = "o";
#endif
    std::string ext = kul::env::GET("MKN_LIB_EXT");
    if (!ext.empty())
      evs["MKN_LIB_EXT"] = ext;
  }

public:
  const std::string& args() const { return aa; }
  void args(const std::string& aa) { this->aa = aa; }

  const std::string& runArgs() const { return ra; }
  void runArgs(const std::string& ra) { this->ra = ra; }

  const kul::hash::map::S2S& jargs() const { return jas; }
  void jargs(const std::string& a, const std::string& b) { jas[a] = b; }

  const std::string& linker() const { return la; }
  void linker(const std::string& la) { this->la = la; }

  const std::string& allinker() const { return al; }
  void allinker(const std::string& al) { this->al = al; }

  const bool& dryRun() const { return this->dr; }
  void dryRun(const bool& dr) { this->dr = dr; }

  const bool& quiet() const { return this->q; }
  void quiet(const bool& q) { this->q = q; }

  const bool& update() const { return this->u; }
  void update(const bool& u) { this->u = u; }

  const bool& fupdate() const { return this->f; }
  void fupdate(const bool& f) { this->f = f; }

  const bool& show() const { return this->s; }
  void show(const bool& s) { this->s = s; }

  const bool& shar() const { return this->sh; }
  void shar(const bool& sh) { this->sh = sh; }

  const bool& stat() const { return this->st; }
  void stat(const bool& st) { this->st = st; }

  const std::string* dependencyString() const { return dep; }
  void dependencyString(const std::string* dep) { this->dep = dep; }

  const uint16_t& debug() const { return de; }
  void debug(const uint16_t& de) { this->de = de; }

  const uint16_t& optimise() const { return op; }
  void optimise(const uint16_t& op) { this->op = op; }

  const uint16_t& warn() const { return wa; }
  void warn(const uint16_t& wa) { this->wa = wa; }

  const uint16_t& dependencyLevel() const { return dl; }
  void dependencyLevel(const uint16_t& dl) { this->dl = dl; }

  const uint16_t& threads() const { return ts; }
  void threads(const uint16_t& t) { this->ts = t; }

  const kul::hash::map::S2S& properkeys() const { return pks; }
  void properkeys(const std::string& k, const std::string& v) { pks[k] = v; }

  const kul::hash::map::S2S& envVars() const { return evs; }
  void envVars(const std::string& k, const std::string& v) { evs[k] = v; }

  void command(const std::string& s) { cmds.insert(s); }
  const kul::hash::set::String& commands() const { return cmds; }

  void modulePhase(const std::string& s) { modPhases.insert(s); }
  const kul::hash::set::String& modulePhases() const { return modPhases; }

  const std::string& with() const { return wi; }
  void with(const std::string& wi) { this->wi = wi; }

  const std::string& without() const { return wo; }
  void without(const std::string& wo) { this->wo = wo; }


  const kul::hash::set::String& withoutParsed() const { return wop; }
  void withoutParsed(const kul::hash::set::String& wop) { this->wop = wop; }

  static AppVars& INSTANCE()
  {
    static AppVars instance;
    return instance;
  }
};

} // namespace maiken
#endif /* _MAIKEN_GLOBALS_HPP_ */