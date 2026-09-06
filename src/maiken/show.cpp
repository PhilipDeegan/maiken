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
#include "maiken.hpp"

namespace {
// Mirrors mkn::kul::lang::cpp::GccCompiler::compileSource's resolution of the
// binary actually invoked for a file type: config prefix/binary, CC/CXX
// env-overridden, wrapper prefix (e.g. ccache) taking precedence if present.
std::string resolve_compiler_binary(YAML::Node const& c) {
  if (!c[maiken::Constants::STR_COMPILER]) return "";
  auto const id =
      maiken::Compilers::INSTANCE().identify(c[maiken::Constants::STR_COMPILER].Scalar());
  std::string const fileType =
      mkn::kul::String::SPLIT(c[maiken::Constants::STR_TYPE].Scalar(), ':')[0];
  std::string const overridden = mkn::kul::String::NO_CASE_CMP(fileType, "c")
                                      ? mkn::kul::lang::cpp::CCompiler::CC(id.binary)
                                      : mkn::kul::lang::cpp::CCompiler::CXX(id.binary);
  return id.prefix.empty() ? overridden : id.prefix[0];
}

std::string first_token(std::string const& s) { return mkn::kul::String::SPLIT(s, " ")[0]; }

// Finds bin (a plain binary name, e.g. "g++") on one of dirs, stripping a
// trailing ".exe" before comparing. Returns its full path, or "" if bin is
// empty or not found.
std::string find_on_path(std::vector<std::string> const& dirs, std::string const& bin) {
  if (bin.empty()) return "";
  for (auto const& d : dirs) {
    mkn::kul::Dir dir(d);
    if (!dir) continue;
    for (auto const& f : dir.files()) {
      std::string const b = (f.name().size() > 3 && f.name().substr(f.name().size() - 4) == ".exe")
                                 ? f.name().substr(0, f.name().size() - 4)
                                 : f.name();
      if (b == bin) return f.full();
    }
  }
  return "";
}

struct BinaryRow {
  std::string type, archiver, compiler, linker;
};

std::string pad(std::string s, size_t w) {
  mkn::kul::String::PAD(s, static_cast<uint16_t>(w));
  return s;
}
}  // namespace

void maiken::Application::showConfig(bool force) {
  if (AppVars::INSTANCE().show() || AppVars::INSTANCE().dryRun()) return;
  if (mkn::kul::LogMan::INSTANCE().inf() || force) {
    std::string repo = (*AppVars::INSTANCE().properkeys().find("MKN_REPO")).second;
    using namespace mkn::kul::cli;
    KOUT(NON) << "+++++++++ BUILD INFO +++++++++";
    KOUT(NON) << "REPO    : " << repo;
    KOUT(NON) << "THREADS : " << AppVars::INSTANCE().threads() << "\n";
    KOUT(NON) << "BINARIES";

    std::string path = mkn::kul::env::GET("PATH");
    {
      auto it(std::find_if(evs.begin(), evs.end(), [](mkn::kul::env::Var const& ev) {
        return strcmp(ev.name(), "PATH") == 0;
      }));
      if (it != evs.end()) path = (*it).toString();
    }

    std::vector<std::string> ps;
    mkn::kul::String::SPLIT(path, mkn::kul::env::SEP(), ps);

    std::vector<BinaryRow> rows;
    size_t typeW = 4, archW = 8, compW = 8;
    for (auto const& c : Settings::INSTANCE().root()[STR_FILE]) {
      BinaryRow row;
      row.type = c[STR_TYPE].Scalar();
      if (c[STR_ARCHIVER]) row.archiver = find_on_path(ps, first_token(c[STR_ARCHIVER].Scalar()));
      row.compiler = find_on_path(ps, resolve_compiler_binary(c));
      if (c[STR_LINKER]) row.linker = find_on_path(ps, first_token(c[STR_LINKER].Scalar()));
      typeW = std::max(typeW, row.type.size());
      archW = std::max(archW, row.archiver.size());
      compW = std::max(compW, row.compiler.size());
      rows.push_back(std::move(row));
    }

    KOUT(NON) << pad("TYPE", typeW) << "  " << pad("ARCHIVER", archW) << "  "
              << pad("COMPILER", compW) << "  "
              << "LINKER";
    for (auto const& row : rows)
      KOUT(NON) << pad(row.type, typeW) << "  " << pad(row.archiver, archW) << "  "
                << pad(row.compiler, compW) << "  " << row.linker;
    if (mkn::kul::LogMan::INSTANCE().dbg()) {
      KOUT(NON) << "ENV     :";
      for (auto const& ev : AppVars::INSTANCE().envVars())
        if (ev.first.find("MKN_") != 0) KOUT(NON) << "  " << ev.first << " = " << ev.second;
      for (auto const& ev : evs) KOUT(NON) << "  " << ev.name() << " = " << ev.toString();
      for (auto const& ev : {"CC", "CXX"})
        if (mkn::kul::env::EXISTS(ev)) KOUT(NON) << "  " << ev << " = " << mkn::kul::env::GET(ev);
    }
    KOUT(NON) << "+++++++++++++++++++++++++++++";
  }
  AppVars::INSTANCE().show(1);
}

void maiken::Application::showHelp() {
  std::vector<std::string> ss = {MKN_DEFS_CMD,     MKN_DEFS_BUILD,  //
                                 MKN_DEFS_CLEAN,   MKN_DEFS_COMP,    MKN_DEFS_DBG,
                                 MKN_DEFS_INIT,    MKN_DEFS_LINK,    MKN_DEFS_PACK,
                                 MKN_DEFS_PROFS,   MKN_DEFS_RUN,     MKN_DEFS_INC,
                                 MKN_DEFS_SRC,     MKN_DEFS_TREE,    "",  //
                                 MKN_DEFS_ARG,     MKN_DEFS_ARGS,    MKN_DEFS_ADD,
                                 MKN_DEFS_BINC,    MKN_DEFS_BPATH,   MKN_DEFS_DIRC,
                                 MKN_DEFS_DEPS,    MKN_DEFS_DUMP,    MKN_DEFS_DEBUG,
                                 MKN_DEFS_GET,     MKN_DEFS_EVSA,    MKN_DEFS_FINC,
                                 MKN_DEFS_FPATH,   MKN_DEFS_HELP,    MKN_DEFS_JARG,
                                 MKN_DEFS_STATIC,  MKN_DEFS_MOD,     MKN_DEFS_MAIN,
                                 MKN_DEFS_NODES,   MKN_DEFS_LINKER,  MKN_DEFS_ALINKR,
                                 MKN_DEFS_OUT,     MKN_DEFS_OPTIM,   MKN_DEFS_PROF,
                                 MKN_DEFS_PROP,    MKN_DEFS_QUIET,   MKN_DEFS_RUN_ARGS,
                                 MKN_DEFS_DRYR,    MKN_DEFS_STAT,    MKN_DEFS_SHARED,
                                 MKN_DEFS_THREDS,  MKN_DEFS_WITHOUT, MKN_DEFS_UPDATE,
                                 MKN_DEFS_FUPDATE, MKN_DEFS_VERSON,
                                 MKN_DEFS_WITH,  //
                                 MKN_DEFS_WARN,    MKN_DEFS_SETTNGS, "",
                                 MKN_DEFS_EXMPL,   MKN_DEFS_EXMPL1,  MKN_DEFS_EXMPL2,
                                 MKN_DEFS_EXMPL3,  MKN_DEFS_EXMPL4,  ""};
  for (auto const& s : ss) KOUT(NON) << s;
}

void maiken::Application::showProfiles() {
  std::vector<std::string> ss;
  size_t b = 0, o = 0;
  for (auto const& n : this->project().root()[STR_PROFILE]) {
    b = n[STR_NAME].Scalar().size() > b ? n[STR_NAME].Scalar().size() : b;
    o = n["os"] ? n["os"].Scalar().size() > o ? n["os"].Scalar().size() : o : o;
  }
  for (auto const& n : this->project().root()[STR_PROFILE]) {
    std::string s(n[STR_NAME].Scalar());
    mkn::kul::String::PAD(s, b);
    std::string os(n["os"] ? "(" + n["os"].Scalar() + ")" : "");
    if (!os.empty()) mkn::kul::String::PAD(os, o);
    std::stringstream s1;
    s1 << "\t" << s << os;
    if (n[STR_PARENT])
      s1 << "\t" << MKN_PARENT << ": " << Properties::RESOLVE(*this, n[STR_PARENT].Scalar());
    ss.push_back(s1.str());
  }
  KOUT(NON) << MKN_PROFILE;
  for (auto const& s : ss) KOUT(NON) << s;
}

void maiken::Application::showTree() const {
  KOUT(NON) << "++++++++ PROJECT TREE ++++++++";
  KOUT(NON) << " " << this->project().root()[STR_NAME].Scalar() << "["
            << (this->p.empty() ? "@" : this->p) << "]";
  showTreeRecursive(1);
  KOUT(NON) << "++++++++++++++++++++++++++++++";
}

void maiken::Application::showTreeRecursive(uint8_t i) const {
  for (auto* const d : deps) {
    std::stringstream ss;
    for (size_t r = 0; r < i; r++) ss << "+";
    KOUT(NON) << " " << ss.str() << " " << d->project().root()[STR_NAME].Scalar() << "["
              << (d->p.empty() ? "@" : d->p) << "]";
    d->showTreeRecursive((++i)--);
  }
}