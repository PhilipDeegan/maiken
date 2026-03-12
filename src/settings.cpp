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

#include "maiken/env.hpp"
#include "maiken/settings.hpp"

#include <optional>

std::unique_ptr<maiken::Settings> maiken::Settings::instance;

namespace maiken {
class SuperSettings {
  friend class maiken::Settings;

 private:
  mkn::kul::hash::set::String files;
  static SuperSettings& INSTANCE() {
    static SuperSettings instance;
    return instance;
  }
  void cycleCheck(std::string const& file) KTHROW(maiken::SettingsException) {
    if (files.count(file))
      KEXCEPT(maiken::SettingsException, "Super cycle detected in file: " + file);
    files.insert(file);
  }
};
}  // namespace maiken

//

std::optional<std::string> maiken::Settings::local_dep_repo() const {
  return getFirstFound(
      [](auto const& s) { return s[STR_LOCAL] && s[STR_LOCAL][STR_REPO]; },
      [](auto const& s) { return mkn::kul::Dir(s[STR_LOCAL][STR_REPO].Scalar()).real(); });
}

std::optional<std::string> maiken::Settings::local_mod_repo() const {
  return getFirstFound(
      [](auto const& s) { return s[STR_LOCAL] && s[STR_LOCAL][STR_MOD_REPO]; },
      [](auto const& s) { return mkn::kul::Dir(s[STR_LOCAL][STR_MOD_REPO].Scalar()).real(); });
}

maiken::Settings::Settings(std::string const& file_) : mkn::kul::yaml::File(file_) {
  KLOG(TRC) << file_;

  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, file_) << " error: local/repo is not a valid directory";
  }
  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_MOD_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_MOD_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, file_) << " error: local/mod-repo is not a valid directory";
  }
  if (root()[STR_REMOTE] && root()[STR_REMOTE][STR_REPO])
    for (auto const& s : mkn::kul::String::SPLIT(root()[STR_REMOTE][STR_REPO].Scalar(), ' '))
      rrs.push_back(s);
  {
    std::string const& rr = _MKN_REMOTE_REPO_;
    for (auto const& s : mkn::kul::String::SPLIT(rr, ' ')) rrs.push_back(s);
  }

  if (root()[STR_REMOTE] && root()[STR_REMOTE][STR_MOD_REPO])
    for (auto const& s : mkn::kul::String::SPLIT(root()[STR_REMOTE][STR_MOD_REPO].Scalar(), ' '))
      rms.push_back(s);
  {
    std::string const& rr = _MKN_REMOTE_MOD_;
    for (auto const& s : mkn::kul::String::SPLIT(rr, ' ')) rms.push_back(s);
  }

  if (root()[STR_SUPER]) {
    auto const f = RESOLVE(root()[STR_SUPER].Scalar(), this);
    if (!f) KEXCEPT(SettingsException, "super file not found: " + file());
    if (f.real() == mkn::kul::File(file()).real())
      KEXCEPT(SettingsException, "super cannot reference itself: " + file());
    SuperSettings::INSTANCE().cycleCheck(f.real());
    sup = std::make_unique<Settings>(
        mkn::kul::yaml::File::CREATE<Settings>(f.full()));  // -> getOrCreate
    POST_CONSTRUCT(sup.get());
    for (auto const& [a, b] : sup->properties())
      if (!ps.count(a)) ps.insert(a, b);
  }
  if (root()[STR_COMPILER] && root()[STR_COMPILER][STR_MASK])
    for (auto const& k : Compilers::INSTANCE().keys())
      if (root()[STR_COMPILER][STR_MASK][k])
        for (auto const& s :
             mkn::kul::String::SPLIT(root()[STR_COMPILER][STR_MASK][k].Scalar(), ' '))
          Compilers::INSTANCE().addMask(s, k);
}

maiken::Settings& maiken::Settings::INSTANCE() KTHROW(mkn::kul::Exit) {
  if (!instance.get()) {
    mkn::kul::File const f("settings.yaml", mkn::kul::user::home("maiken"));
    if (!f.dir().is()) f.dir().mk();
    if (!f.is()) write(f);
    instance = std::make_unique<Settings>(mkn::kul::yaml::File::CREATE<Settings>(f.full()));
  }
  return *instance.get();
}

void maiken::Settings::resolveProperties() KTHROW(SettingsException) {
  ps.setDeletedKey("--DELETED--");
  for (YAML::const_iterator it = root()[STR_PROPERTY].begin(); it != root()[STR_PROPERTY].end();
       ++it)
    ps[it->first.as<std::string>()] = it->second.as<std::string>();
  for (YAML::const_iterator it = root()[STR_PROPERTY].begin(); it != root()[STR_PROPERTY].end();
       ++it) {
    std::string s = Properties::RESOLVE(*this, it->second.as<std::string>());
    if (ps.count(it->first.as<std::string>())) ps.erase(it->first.as<std::string>());
    ps[it->first.as<std::string>()] = s;
  }
}

mkn::kul::File maiken::Settings::RESOLVE(std::string const& s, Settings const* settings)
    KTHROW(SettingsException) {
  using File_t = mkn::kul::File;

  mkn::kul::Dir const cwd{mkn::kul::env::CWD()};
  auto file = [&]() -> std::optional<File_t> {
    auto const dirs =
        settings ? std::vector{cwd, File_t{settings->file()}.dir(), mkn::kul::user::home("maiken")}
                 : std::vector{cwd, mkn::kul::user::home("maiken")};
    for (auto const& dir : dirs) {
      if (auto f = File_t{s, dir}; f) return f;
      for (auto const& suffix : {".yml", ".yaml"})
        if (auto f = File_t{s + suffix, dir}; f) return f;
    }
    return std::nullopt;
  }();

  if (!file) KEXCEPT(SettingsException, "super file not found: " + s);

  return *file;
}

bool maiken::Settings::SET(std::string const& s) {
  auto const file(RESOLVE(s));
  if (file) {
    instance = std::make_unique<Settings>(mkn::kul::yaml::File::CREATE<Settings>(file.real()));
    return 1;
  }
  return 0;
}

mkn::kul::cli::EnvVar maiken::Settings::PARSE_ENV_NODE(YAML::Node const& n,
                                                       Settings const& settings) {
  return maiken::PARSE_ENV_NODE(n, settings, "settings file");
}

void maiken::Settings::POST_CONSTRUCT(Settings* settings_ptr) {
  auto& settings = settings_ptr != nullptr ? *settings_ptr : Settings::INSTANCE();
  auto& root = settings.root();

  if (root[STR_ENV]) {
    if (root[STR_ENV].IsScalar()) {
      auto lines = mkn::kul::String::LINES(root[STR_ENV].Scalar());
      for (auto const& line : lines) {
        auto copy = decltype(root[STR_ENV]){line};
        auto ev = maiken::Settings::PARSE_ENV_NODE(copy, settings);
        mkn::kul::env::SET(ev.name(), ev.toString().c_str());
      }
    } else {
      for (auto const& c : root[STR_ENV]) {
        auto ev = maiken::Settings::PARSE_ENV_NODE(c, settings);
        mkn::kul::env::SET(ev.name(), ev.toString().c_str());
      }
    }
  }

  settings.resolveProperties();
  if (!settings_ptr) settings.resolveFindables();
}
