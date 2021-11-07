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

#include "mkn/kul/map.hpp"

#include "maiken/settings.hpp"

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>

#include <cereal/archives/portable_binary.hpp>
#endif  // _MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

namespace maiken {

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
namespace dist {
class Post;
class RemoteCommandManager;
}  // namespace dist
#endif  // _MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

class AppVars : public Constants {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  friend class dist::RemoteCommandManager;
  friend class dist::Post;
  friend class ::cereal::access;
#endif  // _MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_
 private:
  bool dr = 0, du = 0, fo = 0, fu = 0, q = 0, s = 0, sh = 0, st = 0, u = 0;
  uint16_t de = -1, dl = 0, op = -1, ts = 1, wa = -1;
  std::string aa, al, dep, la, mo, ra, wi, wo;
  mkn::kul::hash::set::String cmds, wop;
  mkn::kul::hash::map::S2S evs, jas, pks;

  static std::shared_ptr<AppVars> instance;

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  uint16_t no = 0;
#endif  // _MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

  AppVars();

 public:
  std::string const& args() const { return aa; }
  void args(std::string const& aa) { this->aa = aa; }

  bool const& dump() const { return this->du; }
  void dump(bool const& du) { this->du = du; }

  bool const& force() const { return this->fo; }
  void force(bool const& fo) { this->fo = fo; }

  std::string const& runArgs() const { return ra; }
  void runArgs(std::string const& ra) { this->ra = ra; }

  const mkn::kul::hash::map::S2S& jargs() const { return jas; }
  void jargs(std::string const& a, std::string const& b) { jas[a] = b; }

  std::string const& linker() const { return la; }
  void linker(std::string const& la) { this->la = la; }

  std::string const& allinker() const { return al; }
  void allinker(std::string const& al) { this->al = al; }

  bool const& dryRun() const { return this->dr; }
  void dryRun(bool const& dr) { this->dr = dr; }

  bool const& quiet() const { return this->q; }
  void quiet(bool const& q) { this->q = q; }

  bool const& update() const { return this->u; }
  void update(bool const& u) { this->u = u; }

  bool const& fupdate() const { return this->fu; }
  void fupdate(bool const& fu) { this->fu = fu; }

  bool const& show() const { return this->s; }
  void show(bool const& s) { this->s = s; }

  bool const& shar() const { return this->sh; }
  void shar(bool const& sh) { this->sh = sh; }

  bool const& stat() const { return this->st; }
  void stat(bool const& st) { this->st = st; }

  std::string const& dependencyString() const { return dep; }
  void dependencyString(std::string const& dep) { this->dep = dep; }

  uint16_t const& debug() const { return de; }
  void debug(uint16_t const& de) { this->de = de; }

  uint16_t const& optimise() const { return op; }
  void optimise(uint16_t const& op) { this->op = op; }

  uint16_t const& warn() const { return wa; }
  void warn(uint16_t const& wa) { this->wa = wa; }

  uint16_t const& dependencyLevel() const { return dl; }
  void dependencyLevel(uint16_t const& dl) { this->dl = dl; }

  uint16_t const& threads() const { return ts; }
  void threads(uint16_t const& t) { this->ts = t; }

  const mkn::kul::hash::map::S2S& properkeys() const { return pks; }
  void properkeys(std::string const& k, std::string const& v) { pks[k] = v; }

  const mkn::kul::hash::map::S2S& envVars() const { return evs; }
  std::string envVar(std::string const&& k) const { return (*evs.find(k)).second; }
  void envVar(std::string const& k, std::string const& v) { evs[k] = v; }

  void command(std::string const& s) { cmds.insert(s); }
  mkn::kul::hash::set::String const& commands() const { return cmds; }

  std::string const& with() const { return wi; }
  void with(std::string const& wi) { this->wi = wi; }

  std::string const& mods() const { return mo; }
  void mods(std::string const& mo) { this->mo = mo; }

  std::string const& without() const { return wo; }
  void without(std::string const& wo) { this->wo = wo; }

  mkn::kul::hash::set::String const& withoutParsed() const { return wop; }
  void withoutParsed(mkn::kul::hash::set::String const& wop) { this->wop = wop; }

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  uint16_t const& nodes() const { return no; }
  void nodes(uint16_t const& no) { this->no = no; }

  static AppVars& SET(std::shared_ptr<AppVars> update) {
    if (instance == nullptr) instance = std::move(update);
    return *instance;
  }

  template <class Archive>
  void save(Archive& ar) const {
    ar(dr, f, q, s, sh, st, u);

    ar(de, dl, op, ts, wa);
    ar(aa, al, la, ra, wi, wo);
    ar(dep);
    ar(no);

    auto convert_to_std_set =
        [](mkn::kul::hash::set::String const& s) -> std::unordered_set<std::string> {
      std::unordered_set<std::string> ret;
      for (auto const& p : s) ret.emplace(p);
      return ret;
    };
    auto convert_to_std_map =
        [](const mkn::kul::hash::map::S2S& s) -> std::unordered_map<std::string, std::string> {
      std::unordered_map<std::string, std::string> ret;
      for (auto const& p : s) ret.emplace(p.first, p.second);
      return ret;
    };

    ar(convert_to_std_set(cmds), convert_to_std_set(wop));
    ar(convert_to_std_map(evs), convert_to_std_map(jas), convert_to_std_map(pks));
  }
  template <class Archive>
  void load(Archive& ar) {
    ar(dr, f, q, s, sh, st, u);

    ar(de, dl, op, ts, wa);
    ar(aa, al, la, ra, wi, wo);
    ar(dep);
    ar(no);

    auto convert_to_kul_set =
        [](const std::unordered_set<std::string>& s) -> mkn::kul::hash::set::String {
      mkn::kul::hash::set::String ret;
      for (auto const& p : s) ret.insert(p);
      return ret;
    };
    auto convert_to_kul_map =
        [](const std::unordered_map<std::string, std::string>& s) -> mkn::kul::hash::map::S2S {
      mkn::kul::hash::map::S2S ret;
      for (auto const& p : s) ret.insert(p.first, p.second);
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

  static AppVars& INSTANCE() {
    if (instance == nullptr) instance.reset(new AppVars);
    return *instance;
  }
};
}  // namespace maiken

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::AppVars, cereal::specialization::member_load_save)

#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_

#endif /* _MAIKEN_GLOBALS_HPP_ */
