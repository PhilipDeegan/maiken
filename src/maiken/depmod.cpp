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
#include "maiken/scm.hpp"

void maiken::Application::loadDepOrMod(YAML::Node const& node, const mkn::kul::Dir& depOrMod,
                                       bool module) KTHROW(mkn::kul::Exception) {
  KOUT(NON) << MKN_PROJECT_NOT_FOUND << depOrMod;
#ifdef _MKN_DISABLE_SCM_
  KEXIT(1, "dep does not exist and remote retrieval is disabled - path: " + depOrMod.path());
#endif
  if (!node[STR_SCM] && !node[STR_NAME])
    KEXIT(1,
          "dep has no name or scm tag so cannot be resolved automatically from "
          "remote repositories - path: " +
              depOrMod.path());
  mkn::kul::env::CWD(this->project().dir());
  std::string const& tscr(node[STR_SCM] ? Properties::RESOLVE(*this, node[STR_SCM].Scalar())
                                        : node[STR_NAME].Scalar());
  std::string const& v(node[STR_VERSION] ? Properties::RESOLVE(*this, node[STR_VERSION].Scalar())
                                         : "");
  try {
    KOUT(NON) << SCMGetter::GET(depOrMod, tscr, module)
                     ->co(depOrMod.path(), SCMGetter::REPO(depOrMod, tscr, module), v);
  } catch (const mkn::kul::scm::Exception& e) {
    if (node[STR_NAME]) {
      mkn::kul::File version(".mkn/dep/ver/" + node[STR_NAME].Scalar());
      if (version) version.rm();
    }
    std::rethrow_exception(std::current_exception());
  }
  mkn::kul::env::CWD(depOrMod);

  if (_MKN_REMOTE_EXEC_) {
#ifdef _WIN32
    if (mkn::kul::File("mkn.bat").is() &&
        mkn::kul::proc::Call("mkn.bat", AppVars::INSTANCE().envVars()).run())
      KEXIT(1, "ERROR in " + depOrMod.path() + "/mkn.bat");
#else
    if (mkn::kul::File("mkn." + std::string(KTOSTRING(__MKN_KUL_OS__)) + ".sh").is() &&
        mkn::kul::proc::Call("./mkn." + std::string(KTOSTRING(__MKN_KUL_OS__)) + ".sh",
                        AppVars::INSTANCE().envVars())
            .run())
      KEXIT(1, "ERROR in " + depOrMod.path() + "mkn." + std::string(KTOSTRING(__MKN_KUL_OS__)) + ".sh");
    else if (mkn::kul::File("mkn.sh").is() &&
             mkn::kul::proc::Call("./mkn.sh", AppVars::INSTANCE().envVars()).run())
      KEXIT(1, "ERROR in " + depOrMod.path() + "/mkn.sh");
#endif
  }
  mkn::kul::env::CWD(this->project().dir());
}

mkn::kul::Dir maiken::Application::resolveDepOrModDirectory(YAML::Node const& n, bool module) {
  std::string d;
  if (n[STR_LOCAL])
    d = Properties::RESOLVE(*this, n[STR_LOCAL].Scalar());
  else {
    std::string depName{n[STR_NAME].Scalar()};
    std::string name(Properties::RESOLVE(*this, depName));
    d = (*AppVars::INSTANCE().properkeys().find(module ? "MKN_MOD_REPO" : "MKN_REPO")).second;
    try {
      mkn::kul::File verFile(depName, ".mkn/dep/ver");
      auto resolveSCMBranch = [=]() -> std::string {
        if (n[STR_VERSION]) return Properties::RESOLVE(*this, n[STR_VERSION].Scalar());
        if (verFile) return mkn::kul::io::Reader(verFile).readLine();
        {
          auto app = Applications::INSTANCE().getOrNullptr(depName);
          if (app) return app->project().dir().name();
        }
#ifdef _MKN_WITH_MKN_RAM_
        KOUT(NON) << "Attempting branch deduction resolution for: " << depName;
        std::string version;
        if (Github::GET_LATEST(depName, version)) return version;
#endif  //_MKN_WITH_MKN_RAM_

        return "master";
      };
      std::string version(resolveSCMBranch());
      if (version.empty()) {
        KEXIT(1, "Error in file: ") << verFile << "\n\tCannot be empty";
      }
      {
        verFile.rm();
        verFile.dir().mk();
        mkn::kul::io::Writer(verFile) << version;
      }
      if (_MKN_REP_VERS_DOT_) mkn::kul::String::REPLACE_ALL(version, ".", mkn::kul::Dir::SEP());

      if (_MKN_REP_NAME_DOT_) mkn::kul::String::REPLACE_ALL(name, ".", mkn::kul::Dir::SEP());
      d = mkn::kul::Dir::JOIN(d, mkn::kul::Dir::JOIN(name, version));
    } catch (mkn::kul::Exception const& e) {
      KERR << e.debug();
    }
  }
  return mkn::kul::Dir(d);
}

void maiken::Application::popDepOrMod(YAML::Node const& n, std::vector<Application*>& vec,
                                      std::string const& s, bool module, bool with)
    KTHROW(mkn::kul::Exception) {
  auto setApp = [&](Application& app, YAML::Node const& node) {
    if (node[STR_SCM]) app.scr = Properties::RESOLVE(*this, node[STR_SCM].Scalar());
    if (node[STR_VERSION]) app.scv = Properties::RESOLVE(*this, node[STR_VERSION].Scalar());
    if (module && node) {
      if (node[STR_INIT]) this->modInit(&app, node[STR_INIT]);
      if (node[STR_COMPILE]) this->modCompile(&app, node[STR_COMPILE]);
      if (node[STR_LINK]) this->modLink(&app, node[STR_LINK]);
      if (node[STR_TEST]) this->modTest(&app, node[STR_TEST]);
      if (node[STR_PACK]) this->modPack(&app, node[STR_PACK]);
      app.isMod = 1;
    }
  };
  std::vector<std::pair<std::string, std::string>> apps;
  auto lam = [&](auto const& depOrMod) {
    const mkn::kul::Dir& projectDir = resolveDepOrModDirectory(depOrMod, module);
    bool f = false;
    for (Application const* ap : vec)
      if (projectDir == ap->project().dir() && p == ap->p) return;

    maiken::Project const& c(*maiken::Projects::INSTANCE().getOrCreate(projectDir));

    auto withoutThis = [=](std::string const& name, std::string const& pro) {
      for (auto const& wo : AppVars::INSTANCE().withoutParsed()) {
        mkn::kul::hash::set::String profiles;
        std::string proName(wo);
        auto lb(wo.find("["));
        auto rb(wo.find("]"));
        if (lb != std::string::npos) {
          if (rb == std::string::npos || rb < lb) KEXIT(1, "Invalid -t argument format provided");
          std::string profile = proName.substr(lb + 1, rb - lb - 1);
          for (auto const& pr : mkn::kul::String::SPLIT(profile, ",")) profiles.insert(pr);
          proName = proName.substr(0, lb);
        }
        if (name == proName) {
          if (pro.empty() && profiles.count("@")) return true;
          if (profiles.count(pro)) return true;
        }
      }
      return false;
    };

    if (depOrMod[STR_PROFILE]) {
      for (auto p :
           mkn::kul::String::SPLIT(Properties::RESOLVE(*this, depOrMod[STR_PROFILE].Scalar()), ' ')) {
        if (p.empty()) continue;
        f = 0;
        if (p == "@")
          p = "";
        else
          for (auto const& node : c.root()[STR_PROFILE])
            if (node[STR_NAME].Scalar() == p) {
              f = 1;
              break;
            }

        if (!f && !p.empty())
          KEXIT(1, "profile does not exist\n" + p + "\n" + project().dir().path());

        if (!module && (depOrMod[STR_NAME] && withoutThis(depOrMod[STR_NAME].Scalar(), p)))
          continue;
        auto* app = Applications::INSTANCE().getOrCreate(c, p, 0);
        if (!with && !module) app->par = this;
        setApp(*app, depOrMod);
        vec.push_back(app);
        app->addRDep(this);
        apps.push_back(std::make_pair(app->project().dir().path(), app->p));
      }
    } else {
      if (!module && (depOrMod[STR_NAME] && withoutThis(depOrMod[STR_NAME].Scalar(), ""))) return;
      auto* app = Applications::INSTANCE().getOrCreate(c, "", 0);
      if (!with && !module) app->par = this;
      setApp(*app, depOrMod);
      vec.push_back(app);
      app->addRDep(this);
      apps.push_back(std::make_pair(app->project().dir().path(), app->p));
    }
  };

  for (auto const& depOrMod : n[s]) lam(depOrMod);

  if (!module && n[STR_SELF])
    for (auto const& split :
         mkn::kul::String::SPLIT(Properties::RESOLVE(*this, n[STR_SELF].Scalar()), ' ')) {
      auto* app = Applications::INSTANCE().getOrCreate(project(), split);
      app->par = this;
      app->scr = scr;
      vec.push_back(app);
      app->addRDep(this);
      apps.push_back(std::make_pair(app->project().dir().path(), app->p));
    }

  cyclicCheck(apps);

  auto cmds = maiken::AppVars::INSTANCE().commands();
  for (auto* ap : vec) {
    auto& app(*ap);
    if (app.buildDir().path().empty()) {
      mkn::kul::env::CWD(app.project().dir());
      if (app.project().root()[STR_SCM])
        app.scr = Properties::RESOLVE(app, app.project().root()[STR_SCM].Scalar());
      if (app.project().root()[STR_BIN])
        app.bin = Properties::RESOLVE(app, app.project().root()[STR_BIN].Scalar());
      if (module) app.ro = false;
      app.setup();
      if (app.sources().size()) app.buildDir().mk();
      mkn::kul::env::CWD(this->project().dir());
    }
    std::string _path(app.inst ? app.inst.escr() : app.buildDir().escr());
    if (app.sources().size() &&
        std::find(app.paths.begin(), app.paths.end(), _path) == app.paths.end())
      app.paths.push_back(_path);
  }
}
