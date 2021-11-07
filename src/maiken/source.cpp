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
#include "maiken/regex.hpp"

void maiken::Application::addMainLine(std::string const& o) KTHROW(mkn::kul::Exception) {
  std::vector<std::string> v(mkn::kul::String::SPLIT(o, ","));
  if (v.size() == 0 || v.size() > 2) KEXIT(1, "main invalid format\n" + project().dir().path());
  mkn::kul::File f(Properties::RESOLVE(*this, v.front()));
  main_ = Source(f.real(), v.size() == 2 ? Properties::RESOLVE(*this, v[1]) : "");
}

void maiken::Application::addSourceLine(std::string const& s) KTHROW(mkn::kul::Exception) {
  auto check_replace = [&](std::string const& str, bool recurse_dir, std::string args) {
    auto it = std::find_if(srcs.begin(), srcs.end(), [&](const std::pair<Source, bool>& element) {
      return element.first == Source(str);
    });
    if (it != srcs.end()) {
      if (!(*it).first.args().empty())
        KEXCEPT(maiken::Exception, "Source file being added twice: ")
            << str << " " << (*it).first.args();
      if ((*it).first.args().empty() && !args.empty()) srcs.erase(it);
    }
    srcs.emplace_back(Source(str, args), recurse_dir);
  };
  auto do_resolve = [&](std::string const& str, bool recurse_dir = true, std::string args = "") {
    args = Properties::RESOLVE(*this, args);
    std::string pResolved(Properties::RESOLVE(*this, str));
    mkn::kul::Dir d(pResolved);
    if (d.is())
      check_replace(d.real(), recurse_dir, args);
    else {
      mkn::kul::File f(pResolved);
      if (f)
        check_replace(f.real(), false, args);
      else {
        auto regexResolved(Regexer::RESOLVE(pResolved));
        if (regexResolved.empty())
          KEXIT(1, "source does not exist\n" + str + "\n" + project().dir().path());
        for (auto const& item : regexResolved) check_replace(item, false, args);
      }
    }
  };

  std::string o = s;
  mkn::kul::String::TRIM(o);
  if (o.find(',') == std::string::npos) {
    for (auto const& str : mkn::kul::cli::asArgs(o)) do_resolve(str);
  } else {
    std::vector<std::string> v(mkn::kul::String::SPLIT(o, ","));
    mkn::kul::Dir d(Properties::RESOLVE(*this, v[0]));
    size_t max = d ? 3 : 2;
    if (v.size() == 0 || v.size() > max)
      KEXIT(1, "source invalid format\n" + project().dir().path());
    do_resolve(v[0], d && mkn::kul::String::BOOL(v[1]), v.size() == max ? v[max - 1] : "");
  }
}

mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<std::vector<maiken::Source>>>
maiken::Application::sourceMap() const {
  mkn::kul::hash::set::String const iMs = inactiveMains();
  mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<std::vector<maiken::Source>>> sm;

  auto check_replace = [&](std::string const& src, std::string args,
                           std::vector<maiken::Source>& v) {
    auto it = std::find_if(v.begin(), v.end(),
                           [&](Source const& element) { return element.in() == src; });
    if (it != v.end()) {
      args += " " + (*it).args();
      v.erase(it);
    }
    v.emplace_back(src, args);
    it = std::find_if(v.begin(), v.end(),
                      [&](Source const& element) { return element.in() == src; });
  };

  for (auto const& sourceDir : sources()) {
    std::vector<mkn::kul::File> files;
    std::string in(sourceDir.first.in());
    mkn::kul::Dir d(in);
    if (d)
      for (auto const& f : d.files(sourceDir.second)) files.push_back(f);
    else
      files.push_back(in);
    for (mkn::kul::File const& file : files) {
      if (file.name().find(".") == std::string::npos || file.name().substr(0, 1).compare(".") == 0)
        continue;
      std::string const ft = file.name().substr(file.name().rfind(".") + 1);
      if (fs.count(ft) > 0) {
        std::string const& rl(file.real());
        bool a = false;
        for (std::string const& s : iMs) {
          a = rl.compare(s) == 0;
          if (a) break;
        }
        if (!a) check_replace(rl, sourceDir.first.args(), sm[ft][file.dir().real()]);
      }
    }
  }
  return sm;
}

bool maiken::Application::incSrc(mkn::kul::File const& file) const {
  bool c = 1;
  if (_MKN_TIMESTAMPS_) {
    std::string const& rl(file.mini());
    c = !stss.count(rl);
    if (!c) {
      size_t const mod = file.timeStamps().modified();
      if (mod == (*stss.find(rl)).second) {
        for (auto const& i : includes()) {
          mkn::kul::Dir inc(i.first);
          if (itss.count(inc.mini()) && includeStamps.count(inc.mini())) {
            if ((*includeStamps.find(inc.mini())).second != (*itss.find(inc.mini())).second) c = 1;
          } else
            c = 1;
          if (c) break;
        }
      } else
        c = 1;
    }
  }
  return c;
}

std::vector<maiken::Source> maiken::SourceFinder::tests() {
  std::vector<maiken::Source> testV;
  for (auto const& p : app.tests) {
    auto& file = p.first;
    if (app.fs.count(file.substr(file.rfind(".") + 1)) == 0) continue;
    testV.emplace_back(mkn::kul::File(file).real());
  }
  return testV;
}

maiken::SourceFinder::SourceFinder(maiken::Application const& _app)
    : app(_app),
      oType("." + AppVars::INSTANCE().envVars().at("MKN_OBJ")),
      objD(_app.buildDir().join("obj"), 1),
      tmpD(_app.buildDir().join("tmp"), 1) {}

std::vector<std::pair<maiken::Source, std::string>> maiken::SourceFinder::all_sources_from(
    SourceMap const& sources, mkn::kul::hash::set::String& objects, std::vector<mkn::kul::File>& cacheFiles) {
  mkn::kul::os::PushDir pushd(app.project().dir());

  std::vector<std::pair<maiken::Source, std::string>> source_objects;
  auto dryRun = AppVars::INSTANCE().dryRun();

  auto _source = [&](auto& s, auto dir) {
    mkn::kul::File const source(s.in());
    if (!app.incSrc(source)) return;

    mkn::kul::File object(s.object(), dir);
    source_objects.emplace_back(Source(dryRun ? source.esc() : source.escm(), s.args()),
                                dryRun ? object.esc() : object.escm());
  };

  auto handle_source = [&](auto& s, auto dir) {
    mkn::kul::File const source(s.in());
    if (app.main_ && Source(source.real()) == *app.main_) return;

    _source(s, dir);
  };

  for (auto const& ft : sources) {
    auto compiler = maiken::Compilers::INSTANCE().get(app.files().at(ft.first).at(STR_COMPILER));
    if (compiler->sourceIsBin()) {
      for (auto const& kv : ft.second)
        for (auto const& s : kv.second) {
          mkn::kul::File source(s.in());
          objects.insert(source.escm());
          cacheFiles.push_back(source);
        }
    } else {
      for (auto const& kv : ft.second)
        for (auto const& s : kv.second) handle_source(s, objD);
    }
  }

  if (app.main_) _source(*app.main_, tmpD);
  for (auto const& test : tests()) handle_source(test, tmpD);
  return source_objects;
}

void maiken::CompilerValidation::check_compiler_for(
    maiken::Application const& app, maiken::Application ::SourceMap const& sources) {
  for (auto const& ft : sources) {
    try {
      if (!app.files().at(ft.first).count(STR_COMPILER))
        KEXIT(1, "No compiler found for filetype " + ft.first);
      Compilers::INSTANCE().get(app.files().at(ft.first).at(STR_COMPILER));
    } catch (CompilerNotFoundException const& e) {
      KEXIT(1, e.what());
    }
  }
}

std::string maiken::Source::object() const {
  mkn::kul::File const source(m_in);
  std::string const oType = "." + AppVars::INSTANCE().envVars().at("MKN_OBJ");
  std::stringstream ss, os;
  ss << std::hex << std::hash<std::string>()(source.real());
  os << ss.str() << "-" << source.name() << oType;
  return os.str();
}
