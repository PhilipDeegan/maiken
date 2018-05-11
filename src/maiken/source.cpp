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
#include "maiken/source.hpp"
#include "maiken.hpp"
#include "maiken/regex.hpp"

void maiken::Application::addSourceLine(const std::string& s)
    KTHROW(kul::Exception) {
  std::string o = s;
  kul::String::TRIM(o);
  if (o.find(',') == std::string::npos) {
    for (const auto& s : kul::cli::asArgs(o)) {
      std::string pResolved(Properties::RESOLVE(*this, s));
      kul::Dir d(pResolved);
      if (d.is())
        srcs.push_back(std::make_pair(d.real(), true));
      else {
        kul::File f(d.locl());
        if (f)
          srcs.push_back(std::make_pair(f.real(), false));
        else {
          auto regexResolved(Regexer::RESOLVE_REGEX(pResolved));
          if (regexResolved.empty())
            KEXIT(1, "source does not exist\n" + s + "\n" +
                         project().dir().path());
          for (const auto& item : regexResolved)
            srcs.push_back(std::make_pair(item, false));
        }
      }
    }
  } else {
    std::vector<std::string> v;
    kul::String::SPLIT(o, ",", v);
    if (v.size() == 0 || v.size() > 2)
      KEXIT(1, "source invalid format\n" + project().dir().path());
    kul::Dir d(Properties::RESOLVE(*this, v[0]));
    if (d)
      srcs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
    else
      KEXIT(1,
            "source does not exist\n" + v[0] + "\n" + project().dir().path());
  }
}

kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>>
maiken::Application::sourceMap() const {
  const kul::hash::set::String iMs = inactiveMains();
  kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> sm;

  // if(!main.empty()){
  //   kul::File f(kul::Dir(main).locl());
  //   if (!f) f = kul::File(main, project().dir());
  //   if (f.is()){}
  //   else if (!AppVars::INSTANCE().dryRun() && !f.is())
  //     KEXIT(1, "") << "ERROR : main does not exist: " << f;
  //   if (f.is())
  //     sm[f.name().substr(f.name().rfind(".") + 1)][f.dir().real()].insert(
  //         f.real());
  //   else if (!AppVars::INSTANCE().dryRun() && !f.is())
  //     KEXIT(1, "") << "ERROR : main does not exist: " << f;
  // }

  // for(const auto &p : tests){
  //   kul::File f(kul::Dir(p.first).locl());
  //   if (!f) f = kul::File(p.first, project().dir());
  //   if (f.is()){}
  //   else if (!AppVars::INSTANCE().dryRun() && !f.is())
  //     KEXIT(1, "") << "ERROR : test does not exist: " << f;
  //   // if (f.is())
  //   //   sm[f.name().substr(f.name().rfind(".") + 1)][f.dir().real()].insert(
  //   //       f.real());
  //   // else if (!AppVars::INSTANCE().dryRun() && !f.is())
  //   //   KEXIT(1) << "ERROR : main does not exist: " << f;
  // }
  for (const std::pair<std::string, bool>& sourceDir : sources()) {
    std::vector<kul::File> files;
    kul::Dir d(sourceDir.first);
    if (d)
      for (const auto& f : d.files(sourceDir.second)) files.push_back(f);
    else
      files.push_back(sourceDir.first);
    for (const kul::File& file : files) {
      if (file.name().find(".") == std::string::npos ||
          file.name().substr(0, 1).compare(".") == 0)
        continue;
      const std::string ft = file.name().substr(file.name().rfind(".") + 1);
      if (fs.count(ft) > 0) {
        const std::string& rl(file.real());
        bool a = false;
        for (const std::string& s : iMs) {
          a = rl.compare(s) == 0;
          if (a) break;
        }
        if (!a) sm[ft][sourceDir.first].insert(rl);
      }
    }
  }
  return sm;
}

bool maiken::Application::incSrc(const kul::File& file) const {
  bool c = 1;
  if (_MKN_TIMESTAMPS_) {
    const std::string& rl(file.mini());
    c = !stss.count(rl);
    if (!c) {
      const uint& mod = file.timeStamps().modified();
      if (mod == (*stss.find(rl)).second) {
        for (const auto& i : includes()) {
          kul::Dir inc(i.first);
          if (itss.count(inc.mini()) && includeStamps.count(inc.mini())) {
            if ((*includeStamps.find(inc.mini())).second !=
                (*itss.find(inc.mini())).second)
              c = 1;
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

std::vector<std::pair<std::string, std::string>>
maiken::SourceFinder::all_sources_from(
    const kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>>&
        sources,
    kul::hash::set::String& objects, std::vector<kul::File>& cacheFiles) {
  const std::string oType(
      "." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
  kul::Dir objD(app.buildDir().join("obj"));
  std::vector<std::pair<std::string, std::string>> source_objects;
  for (const std::pair<std::string,
                       kul::hash::map::S2T<kul::hash::set::String>>& ft :
       sources) {
    const maiken::Compiler* compiler = maiken::Compilers::INSTANCE().get(
        (*(*app.files().find(ft.first)).second.find(STR_COMPILER)).second);
    if (compiler->sourceIsBin()) {
      for (const std::pair<std::string, kul::hash::set::String>& kv : ft.second)
        for (const std::string& s : kv.second) {
          kul::File source(s);
          objects.insert(source.escm());
          cacheFiles.push_back(source);
        }
    } else {
      objD.mk();
      for (const std::pair<std::string, kul::hash::set::String>& kv :
           ft.second) {
        for (const std::string& s : kv.second) {
          const kul::File source(s);
          if (!app.incSrc(source)) continue;
          std::stringstream ss, os;
          ss << std::hex << std::hash<std::string>()(source.real());
          os << ss.str() << "-" << source.name() << oType;
          kul::File object(os.str(), objD);
          source_objects.emplace_back(std::make_pair(
              AppVars::INSTANCE().dryRun() ? source.esc() : source.escm(),
              AppVars::INSTANCE().dryRun() ? object.esc() : object.escm()));
        }
      }
    }
  }
  return source_objects;
}

void maiken::CompilerValidation::check_compiler_for(
    const maiken::Application& app,
    const kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>>&
        sources) {
  for (const std::pair<std::string,
                       kul::hash::map::S2T<kul::hash::set::String>>& ft :
       sources) {
    try {
      if (!(*app.files().find(ft.first)).second.count(STR_COMPILER))
        KEXIT(1, "No compiler found for filetype " + ft.first);
      Compilers::INSTANCE().get(
          (*(*app.files().find(ft.first)).second.find(STR_COMPILER)).second);
    } catch (const CompilerNotFoundException& e) {
      KEXIT(1, e.what());
    }
  }
}
