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
#ifndef _MAIKEN_SETTINGS_HPP_
#define _MAIKEN_SETTINGS_HPP_

#include "mkn/kul/os.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/yaml.hpp"
#include "mkn/kul/except.hpp"

#include "maiken/defs.hpp"

#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace maiken {

class SettingsException : public mkn::kul::Exception {
 public:
  SettingsException(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class Settings : public mkn::kul::yaml::File, public Constants {
  using Findables = mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<std::vector<std::string>>>;

 public:
  Settings(std::string const& s);

  Settings const* super() const { return sup.get(); }

  mkn::kul::yaml::Validator validator() const;
  std::vector<std::string> const& remoteModules() const { return rms; }
  std::vector<std::string> const& remoteRepos() const { return rrs; }
  mkn::kul::hash::map::S2S const& properties() const { return ps; }
  Findables const& findables() const { return findables_; }
  std::optional<std::string> local_dep_repo() const;
  std::optional<std::string> local_mod_repo() const;

  static Settings& INSTANCE() KTHROW(mkn::kul::Exit);
  static bool SET(std::string const& s);
  static mkn::kul::File RESOLVE(std::string const& s, Settings const* settings = nullptr)
      KTHROW(SettingsException);
  static void POST_CONSTRUCT(Settings* settings = nullptr);
  static mkn::kul::cli::EnvVar PARSE_ENV_NODE(YAML::Node const&, Settings const&);

  template <typename If>
  auto getFirstFound(If const _if) const;

  template <typename If, typename Get>
  auto getFirstFound(If const _if, Get const get) const;

  void traverse(auto const fn) const;

  auto operator[](std::string const& s) { return root()[s]; }
  auto operator[](std::string const& s) const { return root()[s]; }

 private:
  std::vector<std::string> rrs, rms;
  std::unique_ptr<Settings> sup;
  mkn::kul::hash::map::S2S ps;
  Findables findables_;

  void resolveProperties() KTHROW(SettingsException);
  void resolveFindables() KTHROW(SettingsException);
  static std::unique_ptr<Settings> instance;
  static void write(mkn::kul::File const& f) KTHROW(mkn::kul::Exit);
};

void Settings::traverse(auto const fn) const {
  using FuncType = std::function<void(Settings const&)>;

  FuncType const loop = [&](auto const& settings) {
    fn(settings);
    if (settings.sup) loop(*settings.sup);
  };

  loop(*this);
}

template <typename If>
auto Settings::getFirstFound(If const _if) const {
  using FuncType = std::function<std::optional<YAML::Node>(Settings const&)>;

  FuncType const loop = [&](auto const& settings) -> std::optional<YAML::Node> {
    if (auto const node = _if(settings)) return node;
    if (settings.sup) return loop(*settings.sup);
    return std::nullopt;
  };

  return loop(*this);
}

template <typename If, typename Get>
auto Settings::getFirstFound(If const _if, Get const get) const {
  using GetRet = std::invoke_result_t<Get, Settings const&>;
  using FuncType = std::function<std::optional<GetRet>(Settings const&)>;

  FuncType const loop = [&](auto const& settings) -> std::optional<GetRet> {
    if (_if(settings)) return get(settings);
    if (settings.sup) return loop(*settings.sup);
    return std::nullopt;
  };

  return loop(*this);
}

}  // namespace maiken

#endif /* _MAIKEN_SETTINGS_HPP_ */
