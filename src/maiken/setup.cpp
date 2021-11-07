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
#include "maiken/github.hpp"
#include "maiken/regex.hpp"

void maiken::Application::setup() KTHROW(mkn::kul::Exception) {
  if (scr.empty() && project().root()[STR_SCM])
    scr = Properties::RESOLVE(*this, project().root()[STR_SCM].Scalar());
  if (AppVars::INSTANCE().update() || AppVars::INSTANCE().fupdate()) {
    scmUpdate(AppVars::INSTANCE().fupdate());
    Projects::INSTANCE().reload(proj);
  }
  setSuper();
  if (scr.empty()) scr = project().root()[STR_NAME].Scalar();

  this->resolveProperties();
  this->preSetupValidation();
  std::string buildD = mkn::kul::Dir::JOIN(STR_BIN, p);
  if (p.empty()) buildD = mkn::kul::Dir::JOIN(STR_BIN, STR_BUILD);
  this->bd = mkn::kul::Dir(project().dir().join(buildD));
  std::string profile(p);
  std::vector<YAML::Node> nodes;
  if (profile.empty()) {
    nodes.push_back(project().root());
    profile = project().root()[STR_NAME].Scalar();
  }
  if (project().root()[STR_PROFILE])
    for (std::size_t i = 0; i < project().root()[STR_PROFILE].size(); i++)
      nodes.push_back(project().root()[STR_PROFILE][i]);

  auto getIfMissing = [&](YAML::Node const& n, bool const mod) {
    std::string const& cwd(mkn::kul::env::CWD());
    mkn::kul::Dir projectDir(resolveDepOrModDirectory(n, mod));
    if (!projectDir.is()) loadDepOrMod(n, projectDir, mod);
    mkn::kul::env::CWD(cwd);
  };

  bool c = 1;
#ifndef _MKN_DISABLE_MODULES_
  std::vector<YAML::Node> mod_nodes;
  if (this->ro) modArgs(AppVars::INSTANCE().mods(), mod_nodes, getIfMissing);
  while (c) {
    c = 0;
    for (auto const& n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_MOD]) {
        if (n[STR_MOD].IsScalar()) {
          for (auto const& mod_str : mkn::kul::String::LINES(n[STR_MOD].Scalar()))
            modArgs(mod_str, mod_nodes, getIfMissing);
        } else if (n[STR_MOD].IsSequence()) {
          bool moreso = 1, stringo = 0;
          for (auto const& mod : n[STR_MOD]) {
            if (stringo && !mod.IsScalar()) KEXIT(1, "NO");
            if (mod.IsScalar()) {
              stringo = 1, moreso = 0;
              for (auto const& mod_str : mkn::kul::String::LINES(mod.Scalar()))
                modArgs(mod_str, mod_nodes, getIfMissing);
            }
          }
          if (moreso) {
            for (auto const& mod : n[STR_MOD]) getIfMissing(mod, 1);
            popDepOrMod(n, modDeps, STR_MOD, 1);
          }
        } else {
          KEXIT(1, "NO");
        }
      }
      if (n[STR_IF_MOD] && n[STR_IF_MOD][KTOSTRING(__MKN_KUL_OS__)]) {
        for (auto const& mod : n[STR_IF_MOD][KTOSTRING(__MKN_KUL_OS__)]) getIfMissing(mod, 1);
        popDepOrMod(n[STR_IF_MOD], modDeps, KTOSTRING(__MKN_KUL_OS__), 1);
      }
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }
  {
    YAML::Node mod_node;
    mod_node[STR_MOD] = mod_nodes;
    popDepOrMod(mod_node, modDeps, STR_MOD, 1);
  }

  {
    auto depLevel(AppVars::INSTANCE().dependencyLevel());
    for (auto* mod : modDeps) {
      mod->ig = 0;
      mod->buildDepVec(AppVars::INSTANCE().dependencyString());
    }
    AppVars::INSTANCE().dependencyLevel(depLevel);
  }
#endif

  std::vector<YAML::Node> with_nodes;
  if (this->ro) withArgs(AppVars::INSTANCE().with(), with_nodes, getIfMissing, 0);
  YAML::Node with_node;

  c = 1;
  profile = p.size() ? p : project().root()[STR_NAME].Scalar();
  while (c) {
    c = 0;
    for (auto const& n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_WITH])
        for (auto const& with_str : mkn::kul::cli::asArgs(n[STR_WITH].Scalar()))
          withArgs(with_str, with_nodes, getIfMissing, 0);
      if (n[STR_DEP]) {
        if (n[STR_DEP].IsScalar())
          for (auto const& with_str : mkn::kul::cli::asArgs(n[STR_DEP].Scalar()))
            withArgs(with_str, with_nodes, getIfMissing, 1);
        else if (n[STR_DEP].IsSequence())
          for (auto const& dep : n[STR_DEP]) getIfMissing(dep, 0);
        else
          KEXCEPTION(STR_DEP) << " is invalid type";
      }
      populateMaps(n);
      popDepOrMod(n, deps, STR_DEP, 0);

      if (n[STR_IF_DEP] && n[STR_IF_DEP][KTOSTRING(__MKN_KUL_OS__)]) {
        auto node = n[STR_IF_DEP][KTOSTRING(__MKN_KUL_OS__)];
        if (node.IsScalar()) {
          for (auto const& with_str : mkn::kul::cli::asArgs(node.Scalar()))
            withArgs(with_str, with_nodes, getIfMissing, 1);
        } else if (n[STR_DEP].IsSequence()) {
          for (auto const& dep : n[STR_IF_DEP][KTOSTRING(__MKN_KUL_OS__)]) getIfMissing(dep, 0);
          popDepOrMod(n[STR_IF_DEP], deps, KTOSTRING(__MKN_KUL_OS__), 0);
        } else
          KEXCEPTION(STR_DEP) << " is invalid type";
      }
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }
  with_node[STR_DEP] = with_nodes;
  popDepOrMod(with_node, deps, STR_DEP, 0, 1);

  if (Settings::INSTANCE().root()[STR_INC])
    for (auto const& l : mkn::kul::String::LINES(Settings::INSTANCE().root()[STR_INC].Scalar()))
      for (auto const& s : mkn::kul::cli::asArgs(l))
        if (s.size()) {
          mkn::kul::Dir d(Properties::RESOLVE(*this, s));
          if (d)
            incs.push_back(std::make_pair(d.real(), false));
          else
            KEXIT(1, "include does not exist\n") << d.path() << "\n" << Settings::INSTANCE().file();
        }
  if (Settings::INSTANCE().root()[STR_PATH])
    for (auto const& l : mkn::kul::String::LINES(Settings::INSTANCE().root()[STR_PATH].Scalar()))
      for (auto const& s : mkn::kul::cli::asArgs(l))
        if (s.size()) {
          mkn::kul::Dir d(Properties::RESOLVE(*this, s));
          if (d)
            paths.push_back(d.escr());
          else
            KEXIT(1, "library path does not exist\n") << d.path() << "\n"
                                                      << Settings::INSTANCE().file();
        }

  this->populateMapsFromDependencies();
  std::vector<std::string> fileStrings{STR_ARCHIVER, STR_COMPILER, STR_LINKER};
  for (auto const& c : Settings::INSTANCE().root()[STR_FILE])
    for (std::string const& s : fileStrings)
      for (auto const& t : mkn::kul::String::SPLIT(c[STR_TYPE].Scalar(), ':'))
        if (fs[t].count(s) == 0 && c[s]) fs[t].insert(s, Properties::RESOLVE(*this, c[s].Scalar()));

  this->postSetupValidation();
  profile = p.size() ? p : project().root()[STR_NAME].Scalar();
  bool nm = 1;
  c = 1;

  while (c) {
    c = 0;
    for (auto const& n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_MODE] && nm) {
        this->mode(compiler::mode_from(n[STR_MODE].Scalar()));
        nm = 0;
      }
      if (out.empty() && n[STR_OUT]) out = Properties::RESOLVE(*this, n[STR_OUT].Scalar());
      if (!main_ && n[STR_MAIN]) addMainLine(n[STR_MAIN].Scalar());
      if (tests.empty() && n[STR_TEST]) tests = Project::populate_tests(n[STR_TEST]);
      if (lang.empty() && n[STR_LANG]) lang = n[STR_LANG].Scalar();
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }

  if (!main_ && lang.empty()) resolveLang();
  if (par) {
    if (main_ && lang.empty()) lang = main_->in().substr(main_->in().rfind(".") + 1);
    main_ = {};
  }

  if (nm) {
    if (AppVars::INSTANCE().shar())
      m = compiler::Mode::SHAR;
    else if (AppVars::INSTANCE().stat())
      m = compiler::Mode::STAT;
  }

  profile = p.size() ? p : project().root()[STR_NAME].Scalar();
  c = 1;
  while (c) {
    c = 0;
    for (auto const& n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (inst.path().empty() && n[STR_INSTALL]) {
        inst = mkn::kul::Dir(Properties::RESOLVE(*this, n[STR_INSTALL].Scalar()));
        if (!inst && !inst.mk())
          KEXIT(1, "install tag is not a valid directory\n" + project().dir().path());
        inst = mkn::kul::Dir(inst.real());
      }

      auto ifArgOrLnk = [&](auto const& n, auto const& nName, std::string& var,
                            mkn::kul::hash::map::S2S& cVal) {
        if (n[nName])
          for (YAML::const_iterator it = n[nName].begin(); it != n[nName].end(); ++it) {
            std::string left(it->first.Scalar());
            if (left.find("_") != std::string::npos) {
              if (left.substr(0, left.find("_")) == KTOSTRING(__MKN_KUL_OS__))
                left = left.substr(left.find("_") + 1);
              else
                continue;
            }

            std::stringstream ifArg_ss;
            for (auto const& s : mkn::kul::cli::asArgs(it->second.Scalar()))
              ifArg_ss << Properties::RESOLVE(*this, s) << " ";
            auto ifArg = ifArg_ss.str();

            bool isCVal = 0;
            for (auto const& s : maiken::Compilers::INSTANCE().keys()) {
              isCVal = (left == s);
              if (isCVal) break;
            }
            if (isCVal) {
              cVal[left] = cVal[left] + ifArg;
              continue;
            }
            auto trues = {lang.empty() && left == STR_BIN, !main_ && left == STR_LIB,
                          m == compiler::Mode::SHAR && left == STR_SHARED,
                          m == compiler::Mode::STAT && left == STR_STATIC,
                          left == KTOSTRING(__MKN_KUL_OS__)};

            if (std::any_of(trues.begin(), trues.end(), [](bool b) { return b; })) var += ifArg;
          }
      };

      ifArgOrLnk(n, STR_IF_ARG, arg, cArg);
      ifArgOrLnk(n, STR_IF_LNK, lnk, cLnk);

      auto if_inc_src = [&](auto const& node, auto str, auto fn) {
        try {
          if (node[str])
            for (auto it = node[str].begin(); it != node[str].end(); ++it)
              if (it->first.Scalar() == KTOSTRING(__MKN_KUL_OS__))
                for (auto const& s : mkn::kul::String::LINES(it->second.Scalar())) (this->*fn)(s);
        } catch (const mkn::kul::StringException&) {
          KEXIT(1, std::string(str) + " contains invalid bool value\n" + project().dir().path());
        }
      };

      if_inc_src(n, STR_IF_INC, &maiken::Application::addIncludeLine);
      if_inc_src(n, STR_IF_SRC, &maiken::Application::addSourceLine);

      if (n[STR_IF_LIB])
        for (YAML::const_iterator it = n[STR_IF_LIB].begin(); it != n[STR_IF_LIB].end(); ++it)
          if (it->first.Scalar() == KTOSTRING(__MKN_KUL_OS__))
            for (auto const& s : mkn::kul::String::SPLIT(it->second.Scalar(), ' '))
              if (s.size()) libs.push_back(Properties::RESOLVE(*this, s));

      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }

  {
    mkn::kul::hash::map::S2S n_tests;
    mkn::kul::Dir testsD(buildDir().join("test"));
    for (auto const& pair : tests) {
      auto files = Regexer::RESOLVE(pair.first);
      if (files.empty()) files.emplace_back(pair.first);
      for (auto const& file : files) {
        auto const fileType = file.substr(file.rfind(".") + 1);
        if (fs.count(fileType) > 0) {
          testsD.mk();
          std::string name = mkn::kul::File(file).name();
          n_tests.insert(file, name.substr(0, name.rfind(".")));
        } else
          n_tests.insert(file, file);
      }
    }
    this->tests = n_tests;
  }
}
