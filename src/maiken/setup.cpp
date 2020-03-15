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

void maiken::Application::setup() KTHROW(kul::Exception) {
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
  std::string buildD = kul::Dir::JOIN(STR_BIN, p);
  if (p.empty()) buildD = kul::Dir::JOIN(STR_BIN, STR_BUILD);
  this->bd = kul::Dir(project().dir().join(buildD));
  std::string profile(p);
  std::vector<YAML::Node> nodes;
  if (profile.empty()) {
    nodes.push_back(project().root());
    profile = project().root()[STR_NAME].Scalar();
  }
  if (project().root()[STR_PROFILE])
    for (std::size_t i = 0; i < project().root()[STR_PROFILE].size(); i++)
      nodes.push_back(project().root()[STR_PROFILE][i]);

  auto getIfMissing = [&](const YAML::Node &n, const bool mod) {
    const std::string &cwd(kul::env::CWD());
    kul::Dir projectDir(resolveDepOrModDirectory(n, mod));
    if (!projectDir.is()) loadDepOrMod(n, projectDir, mod);
    kul::env::CWD(cwd);
  };

  bool c = 1;
#ifndef _MKN_DISABLE_MODULES_
  std::vector<YAML::Node> mod_nodes;
  if (this->ro) modArgs(AppVars::INSTANCE().mods(), mod_nodes, getIfMissing);
  while (c) {
    c = 0;
    for (const auto &n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_MOD]) {
        if (n[STR_MOD].IsScalar()) {
          for (const auto &mod_str : kul::String::LINES(n[STR_MOD].Scalar()))
            modArgs(mod_str, mod_nodes, getIfMissing);
        } else if (n[STR_MOD].IsSequence()) {
          bool moreso = 1, stringo = 0;
          for (const auto &mod : n[STR_MOD]) {
            if (stringo && !mod.IsScalar()) KEXIT(1, "NO");
            if (mod.IsScalar()) {
              stringo = 1;
              moreso = 0;
              for (const auto &mod_str : kul::String::LINES(mod.Scalar()))
                modArgs(mod_str, mod_nodes, getIfMissing);
            }
          }
          if (moreso) {
            for (const auto &mod : n[STR_MOD]) getIfMissing(mod, 1);
            popDepOrMod(n, modDeps, STR_MOD, 1);
          }
        } else {
          KEXIT(1, "NO");
        }
      }
      if (n[STR_IF_MOD] && n[STR_IF_MOD][KTOSTRING(__KUL_OS__)]) {
        for (const auto &mod : n[STR_IF_MOD][KTOSTRING(__KUL_OS__)]) getIfMissing(mod, 1);
        popDepOrMod(n[STR_IF_MOD], modDeps, KTOSTRING(__KUL_OS__), 1);
      }
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }
  YAML::Node mod_node;
  mod_node[STR_MOD] = mod_nodes;
  popDepOrMod(mod_node, modDeps, STR_MOD, 1);

  auto depLevel(AppVars::INSTANCE().dependencyLevel());
  for (auto *mod : modDeps) {
    mod->ig = 0;
    mod->buildDepVec(AppVars::INSTANCE().dependencyString());
  }
  AppVars::INSTANCE().dependencyLevel(depLevel);
#endif

  std::vector<YAML::Node> with_nodes;
  withArgs(AppVars::INSTANCE().with(), with_nodes, getIfMissing, this->ro, 0);
  YAML::Node with_node;

  c = 1;
  profile = p.size() ? p : project().root()[STR_NAME].Scalar();
  while (c) {
    c = 0;
    for (const auto &n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_WITH])
        for (const auto &with_str : kul::cli::asArgs(n[STR_WITH].Scalar()))
          withArgs(with_str, with_nodes, getIfMissing, 1, 0);
      if (n[STR_DEP]) {
        if (n[STR_DEP].IsScalar())
          for (const auto &with_str : kul::cli::asArgs(n[STR_DEP].Scalar()))
            withArgs(with_str, with_nodes, getIfMissing, 1, 1);
        else if (n[STR_DEP].IsSequence())
          for (const auto &dep : n[STR_DEP]) getIfMissing(dep, 0);
        else
          KEXCEPTION(STR_DEP) << " is invalid type";
      }
      populateMaps(n);
      popDepOrMod(n, deps, STR_DEP, 0);
      if (n[STR_IF_DEP] && n[STR_IF_DEP][KTOSTRING(__KUL_OS__)]) {
        for (const auto &dep : n[STR_IF_DEP][KTOSTRING(__KUL_OS__)]) getIfMissing(dep, 0);
        popDepOrMod(n[STR_IF_DEP], deps, KTOSTRING(__KUL_OS__), 0);
      }
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }
  with_node[STR_DEP] = with_nodes;
  popDepOrMod(with_node, deps, STR_DEP, 0, 1);

  if (Settings::INSTANCE().root()[STR_INC])
    for (const auto &l : kul::String::LINES(Settings::INSTANCE().root()[STR_INC].Scalar()))
      for (const auto &s : kul::cli::asArgs(l))
        if (s.size()) {
          kul::Dir d(Properties::RESOLVE(*this, s));
          if (d)
            incs.push_back(std::make_pair(d.real(), false));
          else
            KEXIT(1, "include does not exist\n") << d.path() << "\n" << Settings::INSTANCE().file();
        }
  if (Settings::INSTANCE().root()[STR_PATH])
    for (const auto &l : kul::String::LINES(Settings::INSTANCE().root()[STR_PATH].Scalar()))
      for (const auto &s : kul::cli::asArgs(l))
        if (s.size()) {
          kul::Dir d(Properties::RESOLVE(*this, s));
          if (d)
            paths.push_back(d.escr());
          else
            KEXIT(1, "library path does not exist\n") << d.path() << "\n"
                                                      << Settings::INSTANCE().file();
        }

  this->populateMapsFromDependencies();
  std::vector<std::string> fileStrings{STR_ARCHIVER, STR_COMPILER, STR_LINKER};
  for (const auto &c : Settings::INSTANCE().root()[STR_FILE])
    for (const std::string &s : fileStrings)
      for (const auto &t : kul::String::SPLIT(c[STR_TYPE].Scalar(), ':'))
        if (fs[t].count(s) == 0 && c[s]) fs[t].insert(s, Properties::RESOLVE(*this, c[s].Scalar()));

  this->postSetupValidation();
  profile = p.size() ? p : project().root()[STR_NAME].Scalar();
  bool nm = 1;
  c = 1;

  while (c) {
    c = 0;
    for (const auto &n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (n[STR_MODE] && nm) {
        m = n[STR_MODE].Scalar() == STR_STATIC
                ? compiler::Mode::STAT
                : n[STR_MODE].Scalar() == STR_SHARED ? compiler::Mode::SHAR : compiler::Mode::NONE;
        nm = 0;
      }
      if (out.empty() && n[STR_OUT]) out = Properties::RESOLVE(*this, n[STR_OUT].Scalar());
      if (main.empty() && n[STR_MAIN]) main = n[STR_MAIN].Scalar();
      if (tests.empty() && n[STR_TEST]) tests = Project::populate_tests(n[STR_TEST]);
      if (lang.empty() && n[STR_LANG]) lang = n[STR_LANG].Scalar();
      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }

  if (main.empty() && lang.empty()) resolveLang();
  if (par) {
    if (!main.empty() && lang.empty()) lang = main.substr(main.rfind(".") + 1);
    main.clear();
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
    const auto &propK = AppVars::INSTANCE().properkeys();
    for (const auto &n : nodes) {
      if (n[STR_NAME].Scalar() != profile) continue;
      if (inst.path().empty()) {
        if (Settings::INSTANCE().root()[STR_LOCAL] && propK.count("MKN_BIN") && !main.empty())
          inst = kul::Dir((*propK.find("MKN_BIN")).second);
        else if (Settings::INSTANCE().root()[STR_LOCAL] && propK.count("MKN_LIB") && main.empty())
          inst = kul::Dir((*propK.find("MKN_LIB")).second);
        else if (n[STR_INSTALL])
          inst = kul::Dir(Properties::RESOLVE(*this, n[STR_INSTALL].Scalar()));
        if (!inst.path().empty()) {
          if (!inst && !inst.mk())
            KEXIT(1, "install tag is not a valid directory\n" + project().dir().path());
          inst = kul::Dir(inst.real());
        }
      }

      auto ifArgOrLnk = [=](const auto &n, const auto &nName, std::string &var,
                            kul::hash::map::S2S &cVal) {
        if (n[nName])
          for (YAML::const_iterator it = n[nName].begin(); it != n[nName].end(); ++it) {
            std::string left(it->first.Scalar());
            if (left.find("_") != std::string::npos) {
              if (left.substr(0, left.find("_")) == KTOSTRING(__KUL_OS__))
                left = left.substr(left.find("_") + 1);
              else
                continue;
            }

            std::stringstream ifArg_ss;
            for (const auto &s : kul::cli::asArgs(it->second.Scalar()))
              ifArg_ss << Properties::RESOLVE(*this, s) << " ";
            auto ifArg = ifArg_ss.str();

            bool isCVal = 0;
            for (const auto &s : maiken::Compilers::INSTANCE().keys()) {
              isCVal = (left == s);
              if (isCVal) break;
            }
            if (isCVal) {
              cVal[left] = cVal[left] + ifArg;
              continue;
            }
            if (lang.empty() && left == STR_BIN)
              var += ifArg;
            else if (main.empty() && left == STR_LIB)
              var += ifArg;
            if (m == compiler::Mode::SHAR && left == STR_SHARED)
              var += ifArg;
            else if (m == compiler::Mode::STAT && left == STR_STATIC)
              var += ifArg;
            else if (left == KTOSTRING(__KUL_OS__))
              var += ifArg;
          }
      };

      ifArgOrLnk(n, STR_IF_ARG, arg, cArg);
      ifArgOrLnk(n, STR_IF_LNK, lnk, cLnk);

      try {
        if (n[STR_IF_INC])
          for (YAML::const_iterator it = n[STR_IF_INC].begin(); it != n[STR_IF_INC].end(); ++it)
            if (it->first.Scalar() == KTOSTRING(__KUL_OS__))
              for (const auto &s : kul::String::LINES(it->second.Scalar())) addIncludeLine(s);
      } catch (const kul::StringException &) {
        KEXIT(1, "if_inc contains invalid bool value\n" + project().dir().path());
      }
      try {
        if (n[STR_IF_SRC])
          for (YAML::const_iterator it = n[STR_IF_SRC].begin(); it != n[STR_IF_SRC].end(); ++it)
            if (it->first.Scalar() == KTOSTRING(__KUL_OS__))
              for (const auto &s : kul::String::SPLIT(it->second.Scalar(), ' ')) addSourceLine(s);
      } catch (const kul::StringException &) {
        KEXIT(1, "if_src contains invalid bool value\n" + project().dir().path());
      }
      if (n[STR_IF_LIB])
        for (YAML::const_iterator it = n[STR_IF_LIB].begin(); it != n[STR_IF_LIB].end(); ++it)
          if (it->first.Scalar() == KTOSTRING(__KUL_OS__))
            for (const auto &s : kul::String::SPLIT(it->second.Scalar(), ' '))
              if (s.size()) libs.push_back(Properties::RESOLVE(*this, s));

      profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
      c = !profile.empty();
      break;
    }
  }

  {
    kul::hash::map::S2S n_tests;
    kul::Dir testsD(buildDir().join("test"));
    for (const auto pair : tests) {
      auto files = Regexer::RESOLVE_REGEX(pair.first);
      if (files.empty()) files.emplace_back(pair.first);
      for (const auto file : files) {
        const std::string fileType = file.substr(file.rfind(".") + 1);
        if (fs.count(fileType) == 0) {
          n_tests.insert(file, file);
        } else {
          testsD.mk();
          std::string name = kul::File(file).name();
          n_tests.insert(file, name.substr(0, name.rfind(".")));
        }
      }
    }
    this->tests = n_tests;
  }
}
