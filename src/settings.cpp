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

#include "maiken/compiler/compilers.hpp"
#include "maiken/env.hpp"
#include "maiken/settings.hpp"
#include "maiken/defs.hpp"
#include "maiken/property.hpp"
#include "mkn/kul/cli.hpp"
#include "mkn/kul/env.hpp"
#include "mkn/kul/except.hpp"
#include "mkn/kul/lang/cpp/compilers.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/map.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/string.hpp"
#include "mkn/kul/yaml.hpp"
#include "yaml-cpp/yaml.h"

#include <cstddef>
#include <optional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace maiken {

std::unique_ptr<Settings> Settings::instance;

// the files of the settings being loaded, from the first of its chain: a super: file seen before,
//  above it in the chain or under an earlier super: file, is searched there already and left out
class SuperSettings {
  friend class maiken::Settings;

 private:
  mkn::kul::hash::set::String files;
  std::size_t depth = 0;  // settings files being constructed, 0 when none

  static SuperSettings& INSTANCE() {
    static SuperSettings instance;
    return instance;
  }
  // true the first time file is seen while loading one chain
  bool first(std::string const& file) {
    if (files.count(file)) return false;
    files.insert(file);
    return true;
  }

  class Loading {  // one settings file being constructed
   public:
    Loading(std::string const& file) : outermost{INSTANCE().depth == 0} {
      auto& ss = INSTANCE();
      if (ss.depth++ == 0) ss.files.clear();  // a new chain
      ss.first(file);
    }
    bool const outermost;  // the first file of the chain
    ~Loading() { --INSTANCE().depth; }
    Loading(Loading const&) = delete;
    Loading& operator=(Loading const&) = delete;
  };
};

// super: names, a list or whitespace separated
static std::vector<std::string> super_names(YAML::Node const& node) {
  std::vector<std::string> names;
  auto const add = [&](std::string const& s) {
    for (auto const& line : mkn::kul::String::LINES(s))
      for (auto const& name : mkn::kul::cli::asArgs(line))
        if (!name.empty()) names.push_back(name);
  };
  if (node.IsSequence())
    for (auto const& n : node) add(n.Scalar());
  else
    add(node.Scalar());
  return names;
}
}  // namespace maiken

//

std::optional<std::string> maiken::Settings::local_dep_repo() const {
  auto const node = getFirstFound([](auto const& s) -> std::optional<std::string> {
    if (s[STR_LOCAL] && s[STR_LOCAL][STR_REPO]) return s[STR_LOCAL][STR_REPO].Scalar();
    return std::nullopt;
  });
  if (node) return mkn::kul::Dir{*node}.real();
  return std::nullopt;
}

std::optional<std::string> maiken::Settings::local_mod_repo() const {
  auto const node = getFirstFound([](auto const& s) -> std::optional<std::string> {
    if (s[STR_LOCAL] && s[STR_LOCAL][STR_MOD_REPO]) return s[STR_LOCAL][STR_MOD_REPO].Scalar();
    return std::nullopt;
  });
  if (node) return mkn::kul::Dir{*node}.real();
  return std::nullopt;
}

maiken::Settings::Settings(std::filesystem::path const& file_) : mkn::kul::yaml::File(file_) {
  KLOG(TRC) << file_.string();
  SuperSettings::Loading const loading{mkn::kul::File(file()).real()};

  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, file_.string(), " error: local/repo is not a valid directory");
  }
  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_MOD_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_MOD_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, file_.string(), " error: local/mod-repo is not a valid directory");
  }
  if (root()[STR_SUPER])
    for (auto const& name : super_names(root()[STR_SUPER])) {
      auto const f = RESOLVE(name, this);
      if (f.real() == mkn::kul::File(file()).real())
        KEXCEPT(SettingsException, "super cannot reference itself: " + file());
      if (!SuperSettings::INSTANCE().first(f.real())) continue;
      sups.emplace_back(std::make_unique<Settings>(
          mkn::kul::yaml::File::CREATE<Settings>(f.full())));  // -> getOrCreate
    }

  if (loading.outermost) {
    std::vector<Settings*> files;
    preorder(files);
    // env: is the first file's to have one, the files after it leave theirs out
    bool found = false;
    for (auto* f : files) {
      f->envSkipped = found;
      found = found || static_cast<bool>(f->root()[STR_ENV]);
    }
    // from the end of the chain up, each file after the files it extends: their properties, then
    //  its env: and properties; this file's own env: and properties follow in POST_CONSTRUCT
    for (auto it = files.rbegin(); it != files.rend(); ++it) {
      (*it)->mergeSuperProperties();
      if (*it != this) POST_CONSTRUCT(*it);
    }
  }

  // remote: repos from the first file of the chain setting them, then the built-in ones
  auto const remotes = [&](std::string const& key, std::string const& builtin) {
    std::vector<std::string> v;
    auto const found = getFirstFound([&](Settings const& s) -> std::optional<std::string> {
      if (s[STR_REMOTE] && s[STR_REMOTE][key]) return s[STR_REMOTE][key].Scalar();
      return std::nullopt;
    });
    if (found)
      for (auto const& r : mkn::kul::String::SPLIT(*found, ' ')) v.push_back(r);
    for (auto const& r : mkn::kul::String::SPLIT(builtin, ' ')) v.push_back(r);
    return v;
  };
  rrs = remotes(STR_REPO, MKN_REMOTE_REPO);
  rms = remotes(STR_MOD_REPO, MKN_REMOTE_MOD);
  if (root()[STR_COMPILER] && root()[STR_COMPILER][STR_MASK])
    for (auto const& k : Compilers::INSTANCE().keys())
      if (root()[STR_COMPILER][STR_MASK][k])
        for (auto const& s :
             mkn::kul::String::SPLIT(root()[STR_COMPILER][STR_MASK][k].Scalar(), ' '))
          Compilers::INSTANCE().addMask(s, k);
}

void maiken::Settings::preorder(std::vector<Settings*>& files) {
  files.push_back(this);
  for (auto& sup : sups) sup->preorder(files);
}

void maiken::Settings::mergeSuperProperties() {  // the first super: file's first
  for (auto const& sup : sups)
    for (auto const& [a, b] : sup->properties())
      if (!ps.count(a)) ps.insert(a, b);
}

maiken::Settings& maiken::Settings::INSTANCE() KTHROW(mkn::kul::Exit) {
  if (!instance.get()) {
    auto const f = userDefault();
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

  mkn::kul::Dir const cwd{mkn::kul::env::CWD()}, home{mkn::kul::user::home("maiken").string()};
  auto file = [&]() -> std::optional<File_t> {
    bool const is_abs =
        !s.empty() && (s.compare(0, mkn::kul::Dir::SEP().size(), mkn::kul::Dir::SEP()) == 0 ||
                       (s.size() > 1 && s[1] == ':'));
    if (is_abs) {
      if (auto f = File_t{s}; f) return f;
      for (auto const& suffix : {".yml", ".yaml"})
        if (auto f = File_t{s + suffix}; f) return f;
      return std::nullopt;
    }
    auto const dirs =
        settings ? std::vector{cwd, File_t{settings->file()}.dir(), home} : std::vector{cwd, home};
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

void maiken::Settings::SET(std::string const& s) {
  auto const file(RESOLVE(s));
  instance = std::make_unique<Settings>(mkn::kul::yaml::File::CREATE<Settings>(file.real()));
}

mkn::kul::env::Var maiken::Settings::PARSE_ENV_NODE(YAML::Node const& n, Settings const& settings) {
  return maiken::PARSE_ENV_NODE(n, settings, "settings file");
}

std::vector<mkn::kul::env::Var> maiken::Settings::envVars() const {
  std::vector<mkn::kul::env::Var> evs;
  auto const node = root()[STR_ENV];
  if (!node) return evs;
  if (node.IsScalar())
    for (auto const& line : mkn::kul::String::LINES(node.Scalar()))
      evs.emplace_back(PARSE_ENV_NODE(decltype(node){line}, *this));
  else
    for (auto const& c : node) evs.emplace_back(PARSE_ENV_NODE(c, *this));
  return evs;
}

void maiken::Settings::POST_CONSTRUCT(Settings* settings_ptr) {
  auto& settings = settings_ptr != nullptr ? *settings_ptr : Settings::INSTANCE();

  if (!settings.envSkipped)
    for (auto const& ev : settings.envVars()) mkn::kul::env::SET(ev.name(), ev.toString().c_str());

  settings.resolveProperties();
  if (!settings_ptr) settings.resolveFindables();
  if (!settings_ptr) settings.resolveBinaries();
}
