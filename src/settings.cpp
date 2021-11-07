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
#include "maiken.hpp"
#include "maiken/env.hpp"

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

maiken::Settings::Settings(std::string const& file_) : mkn::kul::yaml::File(file_) {
  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, "settings.yaml local/repo is not a valid directory");
  }
  if (root()[STR_LOCAL] && root()[STR_LOCAL][STR_MOD_REPO]) {
    mkn::kul::Dir d(root()[STR_LOCAL][STR_MOD_REPO].Scalar());
    if (!d.is() && !d.mk())
      KEXCEPT(SettingsException, "settings.yaml local/mod-repo is not a valid directory");
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
    mkn::kul::File f(RESOLVE(root()[STR_SUPER].Scalar()));
    if (!f) KEXCEPT(SettingsException, "super file not found\n" + file());
    if (f.real() == mkn::kul::File(file()).real())
      KEXCEPT(SettingsException, "super cannot reference itself\n" + file());
    SuperSettings::INSTANCE().cycleCheck(f.real());
    sup = std::make_unique<Settings>(mkn::kul::yaml::File::CREATE<Settings>(f.full()));
    for (auto const& p : sup->properties())
      if (!ps.count(p.first)) ps.insert(p.first, p.second);
  }
  if (root()[STR_COMPILER] && root()[STR_COMPILER][STR_MASK])
    for (auto const& k : Compilers::INSTANCE().keys())
      if (root()[STR_COMPILER][STR_MASK][k])
        for (auto const& s : mkn::kul::String::SPLIT(root()[STR_COMPILER][STR_MASK][k].Scalar(), ' '))
          Compilers::INSTANCE().addMask(s, k);

  resolveProperties();

}

maiken::Settings& maiken::Settings::INSTANCE() KTHROW(mkn::kul::Exit) {
  if (!instance.get()) {
    mkn::kul::File const f("settings.yaml", mkn::kul::user::home("maiken"));
    if (!f.dir().is()) f.dir().mk();
    if (!f.is()) {
      write(f);
    }
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

std::string maiken::Settings::RESOLVE(std::string const& s) KTHROW(SettingsException) {
  std::vector<mkn::kul::File> pos{mkn::kul::File(s), mkn::kul::File(s + ".yaml"),
                             mkn::kul::File(s, mkn::kul::user::home("maiken")),
                             mkn::kul::File(s + ".yaml", mkn::kul::user::home("maiken"))};
  for (auto const& f : pos)
    if (f.is()) return f.real();

  return "";
}

bool maiken::Settings::SET(std::string const& s) {
  std::string file(RESOLVE(s));
  if (file.size()) {
    instance = std::make_unique<Settings>(mkn::kul::yaml::File::CREATE<Settings>(file));
    return 1;
  }
  return 0;
}

const mkn::kul::yaml::Validator maiken::Settings::validator() const {
  using namespace mkn::kul::yaml;

  std::vector<NodeValidator> masks;
  for (auto const& s : Compilers::INSTANCE().keys())
    masks.push_back(NodeValidator(s, {}, 0, NodeType::STRING));

  return Validator({
    NodeValidator("super"), NodeValidator("property", {NodeValidator("*")}, 0, NodeType::MAP),
        NodeValidator("inc"), NodeValidator("path"),
        NodeValidator("local",
                      {NodeValidator("repo"), NodeValidator("mod-repo"), NodeValidator("debugger")},
                      0, NodeType::MAP),
        NodeValidator("remote", {NodeValidator("repo"), NodeValidator("mod-repo")}, 0,
                      NodeType::MAP),
        NodeValidator("env",
                      {NodeValidator("name", 1), NodeValidator("value", 1), NodeValidator("mode")},
                      0, NodeType::NON),
        NodeValidator("file",
                      {NodeValidator("type", 1), NodeValidator("compiler", 1),
                       NodeValidator("linker"), NodeValidator("archiver")},
                      1, NodeType::LIST),
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
        NodeValidator("dist",
                      {NodeValidator("port"),
                       NodeValidator("nodes",
                                     {NodeValidator("host", 1), NodeValidator("port", 1),
                                      NodeValidator("user"), NodeValidator("pass")},
                                     0, NodeType::LIST)},
                      0, NodeType::MAP),
#endif  // _MKN_WITH_MKN_RAM_ && _MKN_WITH_IO_CEREAL_
        NodeValidator("compiler", {NodeValidator("mask", masks, 0, NodeType::MAP)}, 0,
                      NodeType::MAP)
  });
}

void maiken::Settings::write(mkn::kul::File const& file) KTHROW(mkn::kul::Exit) {
  mkn::kul::io::Writer w(file);
  w.write("\n", true);

  bool c = mkn::kul::env::WHICH("clang") || mkn::kul::env::WHICH("clang.exe");
  bool g = mkn::kul::env::WHICH("gcc") || mkn::kul::env::WHICH("gcc.exe");
  {
    auto gcc_pref(mkn::kul::env::GET("MKN_GCC_PREFERRED"));
    bool use_gcc = gcc_pref.empty() ? 0 : mkn::kul::String::BOOL(gcc_pref);
    if (c && g && use_gcc) c = 0;
  }

  w.write("#local:", true);
  w.write("## Optionnaly override local repository directory", true);
  w.write("#  repo: <directory>", true);
  w << mkn::kul::os::EOL();

#ifdef _WIN32
  auto cl_pref(mkn::kul::env::GET("MKN_CL_PREFERRED"));
  bool use_cl = cl_pref.empty() ? 0 : mkn::kul::String::BOOL(cl_pref);
  if (!use_cl && (c || g)) {
#else
  if (c || g) {
#endif
    w.write("## Add include directories to every compilation", true);
    w.write("#inc: <directory>", true);
    w.write("## Add library paths when linking every binary", true);
    w.write("#path:    <directory>\n", true);
    w << mkn::kul::os::EOL();

    w.write(
        "## Modify environement variables for application commands - excludes "
        "run",
        true);
    w.write("#env:", true);
    w.write("#- name: VAR", true);
    w.write("#  mode: prepend", true);
    w.write("#  value: value", true);
    w << mkn::kul::os::EOL();

    w.write("file:", true);
    w.write("- type: c:S", true);
    w.write("  archiver: ar -cr", true);
    w << "  compiler: " << (c ? "clang" : "gcc") << mkn::kul::os::EOL();
    w << "  linker: " << (c ? "clang" : "gcc") << mkn::kul::os::EOL();
    w.write("- type: cpp:cxx:cc", true);
    w.write("  archiver: ar -cr", true);
    w << "  compiler: " << (c ? "clan" : "") << "g++" << mkn::kul::os::EOL();
    w << "  linker: " << (c ? "clan" : "") << "g++" << mkn::kul::os::EOL();
    w << mkn::kul::os::EOL();
  }

#ifdef _WIN32

  if (use_cl || (!c && !g)) {
    auto cl(mkn::kul::env::WHERE("cl.exe"));
    auto inc(mkn::kul::env::GET("INCLUDE"));
    auto lib(mkn::kul::env::GET("LIB"));

    if (cl.empty() || inc.empty() || lib.empty()) {
      w.flush().close();
      file.rm();
      KEXIT(1, "gcc or clang not found, vcvars not detected")
          << mkn::kul::os::EOL()
          << "\tRun vcvarsall.bat or view mkn wiki to see how to configure "
             "maiken settings.yaml"
          << mkn::kul::os::EOL() << "\t@ https://github.com/mkn/mkn/wiki";
    }

    w.write("inc: ", true);
    for (auto s : mkn::kul::String::SPLIT(inc, mkn::kul::env::SEP())) {
      mkn::kul::String::REPLACE_ALL(s, "\\", "/");
      mkn::kul::String::REPLACE_ALL(s, " ", "\\ ");
      w << "  " << s << mkn::kul::os::EOL();
    }
    w << mkn::kul::os::EOL();

    w.write("path: ", true);
    for (auto s : mkn::kul::String::SPLIT(lib, mkn::kul::env::SEP())) {
      mkn::kul::String::REPLACE_ALL(s, "\\", "/");
      mkn::kul::String::REPLACE_ALL(s, " ", "\\ ");
      w << "  " << s << mkn::kul::os::EOL();
    }
    w << mkn::kul::os::EOL();

    w.write("env:", true);
    w.write("- name: PATH", true);
    w.write("  mode: prepend", true);
    w << "  value: " << mkn::kul::File(cl).dir().real() << mkn::kul::os::EOL();
    w << mkn::kul::os::EOL();

    w.write("file:", true);
    w.write("- type: cpp:cxx:cc:c", true);
    w.write("  archiver: lib", true);
    w.write("  compiler: cl", true);
    w.write("  linker: link", true);
  }
#endif
  w << mkn::kul::os::EOL();

  w.write("# Other examples", true);
  w.write("#- type: cu", true);
  w.write("#  archiver: ar -cr", true);
  w.write("#  compiler: nvcc", true);
  w.write("#  linker: nvcc", true);

  w.write("#- type: m", true);
  w.write("#  archiver: ar -cr", true);
  w.write("#  compiler: g++ -lobjc", true);
  w.write("#  linker: g++", true);

  w.write("#- type: cs", true);
  w.write("#  compiler: csc", true);
  w.write("#  linker: csc", true);
}

mkn::kul::cli::EnvVar maiken::Settings::PARSE_ENV_NODE(YAML::Node const& n, Settings const& settings) {
  return maiken::PARSE_ENV_NODE(n, settings, "settings file");
}

void maiken::Settings::POST_CONSTRUCT() {

  auto& settings = Settings::INSTANCE();
  auto& root = settings.root();
  auto& appVars = AppVars::INSTANCE();

  if (root[STR_ENV]) {
    if (root[STR_ENV].IsScalar()) {
      auto lines = mkn::kul::String::LINES(root[STR_ENV].Scalar());
      for (auto const& line : lines) {
        auto copy = decltype(root[STR_ENV]){line};
        auto ev = maiken::Settings::PARSE_ENV_NODE(copy, settings);
        appVars.envVar(ev.name(), ev.toString());
        mkn::kul::env::SET(ev.name(), ev.toString().c_str());
      }
    } else {
      for (const auto& c : root[STR_ENV]) {
        auto ev = maiken::Settings::PARSE_ENV_NODE(c, settings);
        appVars.envVar(ev.name(), ev.toString());
        mkn::kul::env::SET(ev.name(), ev.toString().c_str());
      }
    }
  }
}
