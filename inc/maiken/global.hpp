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

#include "maiken/settings.hpp"

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>

#include <cereal/archives/portable_binary.hpp>
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

namespace maiken {

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
namespace dist {
class Post;
class RemoteCommandManager;
}  // namespace dist
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

class AppVars : public Constants {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  friend class dist::RemoteCommandManager;
  friend class dist::Post;
  friend class ::cereal::access;
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_
 private:
  bool dr = 0, f = 0, q = 0, s = 0, sh = 0, st = 0, u = 0;
  uint16_t de = -1, dl = 0, op = -1, ts = 1, wa = -1;
  std::string aa, al, dep, la, ra, wi, wo;
  kul::hash::set::String cmds, wop;
  kul::hash::map::S2S evs, jas, pks;

  static std::shared_ptr<AppVars> instance;

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  uint16_t no = 0;
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

  AppVars();

 public:
  const std::string &args() const { return aa; }
  void args(const std::string &aa) { this->aa = aa; }

  const std::string &runArgs() const { return ra; }
  void runArgs(const std::string &ra) { this->ra = ra; }

  const kul::hash::map::S2S &jargs() const { return jas; }
  void jargs(const std::string &a, const std::string &b) { jas[a] = b; }

  const std::string &linker() const { return la; }
  void linker(const std::string &la) { this->la = la; }

  const std::string &allinker() const { return al; }
  void allinker(const std::string &al) { this->al = al; }

  const bool &dryRun() const { return this->dr; }
  void dryRun(const bool &dr) { this->dr = dr; }

  const bool &quiet() const { return this->q; }
  void quiet(const bool &q) { this->q = q; }

  const bool &update() const { return this->u; }
  void update(const bool &u) { this->u = u; }

  const bool &fupdate() const { return this->f; }
  void fupdate(const bool &f) { this->f = f; }

  const bool &show() const { return this->s; }
  void show(const bool &s) { this->s = s; }

  const bool &shar() const { return this->sh; }
  void shar(const bool &sh) { this->sh = sh; }

  const bool &stat() const { return this->st; }
  void stat(const bool &st) { this->st = st; }

  const std::string &dependencyString() const { return dep; }
  void dependencyString(const std::string &dep) { this->dep = dep; }

  const uint16_t &debug() const { return de; }
  void debug(const uint16_t &de) { this->de = de; }

  const uint16_t &optimise() const { return op; }
  void optimise(const uint16_t &op) { this->op = op; }

  const uint16_t &warn() const { return wa; }
  void warn(const uint16_t &wa) { this->wa = wa; }

  const uint16_t &dependencyLevel() const { return dl; }
  void dependencyLevel(const uint16_t &dl) { this->dl = dl; }

  const uint16_t &threads() const { return ts; }
  void threads(const uint16_t &t) { this->ts = t; }

  const kul::hash::map::S2S &properkeys() const { return pks; }
  void properkeys(const std::string &k, const std::string &v) { pks[k] = v; }

  const kul::hash::map::S2S &envVars() const { return evs; }
  std::string envVar(const std::string &&k) const { return (*evs.find(k)).second; }
  void envVar(const std::string &k, const std::string &v) { evs[k] = v; }

  void command(const std::string &s) { cmds.insert(s); }
  const kul::hash::set::String &commands() const { return cmds; }

  const std::string &with() const { return wi; }
  void with(const std::string &wi) { this->wi = wi; }

  const std::string &without() const { return wo; }
  void without(const std::string &wo) { this->wo = wo; }

  const kul::hash::set::String &withoutParsed() const { return wop; }
  void withoutParsed(const kul::hash::set::String &wop) { this->wop = wop; }

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  const uint16_t &nodes() const { return no; }
  void nodes(const uint16_t &no) { this->no = no; }

  static AppVars &SET(std::shared_ptr<AppVars> update) {
    if (instance == nullptr) instance = std::move(update);
    return *instance;
  }

  template <class Archive>
  void save(Archive &ar) const {
    ar(dr, f, q, s, sh, st, u);

    ar(de, dl, op, ts, wa);
    ar(aa, al, la, ra, wi, wo);
    ar(dep);
    ar(no);

    auto convert_to_std_set =
        [](const kul::hash::set::String &s) -> std::unordered_set<std::string> {
      std::unordered_set<std::string> ret;
      for (const auto &p : s) ret.emplace(p);
      return ret;
    };
    auto convert_to_std_map =
        [](const kul::hash::map::S2S &s) -> std::unordered_map<std::string, std::string> {
      std::unordered_map<std::string, std::string> ret;
      for (const auto &p : s) ret.emplace(p.first, p.second);
      return ret;
    };

    ar(convert_to_std_set(cmds), convert_to_std_set(wop));
    ar(convert_to_std_map(evs), convert_to_std_map(jas), convert_to_std_map(pks));
  }
  template <class Archive>
  void load(Archive &ar) {
    ar(dr, f, q, s, sh, st, u);

    ar(de, dl, op, ts, wa);
    ar(aa, al, la, ra, wi, wo);
    ar(dep);
    ar(no);

    auto convert_to_kul_set =
        [](const std::unordered_set<std::string> &s) -> kul::hash::set::String {
      kul::hash::set::String ret;
      for (const auto &p : s) ret.insert(p);
      return ret;
    };
    auto convert_to_kul_map =
        [](const std::unordered_map<std::string, std::string> &s) -> kul::hash::map::S2S {
      kul::hash::map::S2S ret;
      for (const auto &p : s) ret.insert(p.first, p.second);
      return ret;
    };
    std::unordered_set<std::string> _cmds, _wop;
    ar(cmds, _wop);
    cmds = convert_to_kul_set(_cmds);
    wop = convert_to_kul_set(_wop);

    std::unordered_map<std::string, std::string> _evs, _jas, _pks;
    ar(_evs, _jas, _pks);

    evs = convert_to_kul_map(_evs);
    jas = convert_to_kul_map(_jas);
    pks = convert_to_kul_map(_pks);
  }

#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

  static AppVars &INSTANCE() {
    if (instance == nullptr) instance.reset(new AppVars);
    return *instance;
  }
};
}  // namespace maiken

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::AppVars, cereal::specialization::member_load_save)

#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

#endif /* _MAIKEN_GLOBALS_HPP_ */
