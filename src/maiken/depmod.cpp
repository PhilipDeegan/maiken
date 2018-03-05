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

void
maiken::Application::loadDepOrMod(const YAML::Node& node,
                                  const kul::Dir& depOrMod,
                                  bool module) KTHROW(kul::Exception)
{
  KOUT(NON) << MKN_PROJECT_NOT_FOUND << depOrMod;
#ifdef _MKN_DISABLE_SCM_
  KEXIT(1,
        "dep does not exist and remote retrieval is disabled - path: " +
          depOrMod.path());
#endif
  if (!node[STR_SCM] && !node[STR_NAME])
    KEXIT(1,
          "dep has no name or scm tag so cannot be resolved automatically from "
          "remote repositories - path: " +
            depOrMod.path());
  kul::env::CWD(this->project().dir());
  const std::string& tscr(node[STR_SCM]
                            ? Properties::RESOLVE(*this, node[STR_SCM].Scalar())
                            : node[STR_NAME].Scalar());
  const std::string& v(
    node[STR_VERSION] ? Properties::RESOLVE(*this, node[STR_VERSION].Scalar())
                      : "");
  try{
    KOUT(NON) << SCMGetter::GET(depOrMod, tscr, module)
                   ->co(
                     depOrMod.path(), SCMGetter::REPO(depOrMod, tscr, module), v);
  }catch(const kul::scm::Exception& e){
    if(node[STR_NAME]){
      kul::File version(".mkn/dep/ver/"+node[STR_NAME].Scalar());
      if(version) version.rm();
    }
    std::rethrow_exception(std::current_exception());
  }
  kul::env::CWD(depOrMod);

  if (_MKN_REMOTE_EXEC_) {
#ifdef _WIN32
    if (kul::File("mkn.bat").is() &&
        kul::proc::Call("mkn.bat", AppVars::INSTANCE().envVars()).run())
      KEXIT(1, "ERROR in " + depOrMod.path() + "/mkn.bat");
#else
    if (kul::File("mkn." + std::string(KTOSTRING(__KUL_OS__)) + ".sh").is() &&
        kul::proc::Call("./mkn." + std::string(KTOSTRING(__KUL_OS__)) + ".sh",
                        AppVars::INSTANCE().envVars())
          .run())
      KEXIT(1,
            "ERROR in " + depOrMod.path() + "mkn." +
              std::string(KTOSTRING(__KUL_OS__)) + ".sh");
    else if (kul::File("mkn.sh").is() &&
             kul::proc::Call("./mkn.sh", AppVars::INSTANCE().envVars()).run())
      KEXIT(1, "ERROR in " + depOrMod.path() + "/mkn.sh");
#endif
  }
  kul::env::CWD(this->project().dir());
}

kul::Dir
maiken::Application::resolveDepOrModDirectory(const YAML::Node& n, bool module)
{
  std::string d;
  if (n[STR_LOCAL])
    d = Properties::RESOLVE(*this, n[STR_LOCAL].Scalar());
  else {
    d = (*AppVars::INSTANCE().properkeys().find(module ? "MKN_MOD_REPO"
                                                       : "MKN_REPO"))
          .second;
    try {
      kul::File verFile(n[STR_NAME].Scalar(), ".mkn/dep/ver");
      auto resolveSCMBranch = [=]() -> std::string {
        if (n[STR_VERSION])
          return Properties::RESOLVE(*this, n[STR_VERSION].Scalar());
        if (verFile)
          return kul::io::Reader(verFile).readLine();
        {
          auto app =
            Applications::INSTANCE().getOrNullptr(n[STR_NAME].Scalar());
          if (app)
            return app->project().dir().name();
        }
#ifdef _MKN_WITH_MKN_RAM_
        KOUT(NON) << "Attempting branch deduction resolution for: "
          << n[STR_NAME].Scalar();
        std::string version;
        if (Github::GET_LATEST(n[STR_NAME].Scalar(), version))
          return version;
#endif //_MKN_WITH_MKN_RAM_
        return "master";
      };
      std::string version(resolveSCMBranch());
      if (version.empty()) {
        KEXIT(1, "Error in file: ") << verFile << "\n\tCannot be empty";
      }
      {
        verFile.rm();
        verFile.dir().mk();
        kul::io::Writer(verFile) << version;
      }
      if (_MKN_REP_VERS_DOT_)
        kul::String::REPLACE_ALL(version, ".", kul::Dir::SEP());
      std::string name(Properties::RESOLVE(*this, n[STR_NAME].Scalar()));
      if (_MKN_REP_NAME_DOT_)
        kul::String::REPLACE_ALL(name, ".", kul::Dir::SEP());
      d = kul::Dir::JOIN(d, kul::Dir::JOIN(name, version));
    } catch (const kul::Exception& e) {
      KERR << e.debug();
    }
  }
  return kul::Dir(d);
}

void
maiken::Application::popDepOrMod(const YAML::Node& n,
                                 std::vector<Application*>& vec,
                                 const std::string& s,
                                 bool module,
                                 bool with) KTHROW(kul::Exception)
{
  auto setApp = [&](Application& app, const YAML::Node& node) {
    if (node[STR_SCM])
      app.scr = Properties::RESOLVE(*this, node[STR_SCM].Scalar());
    if (node[STR_VERSION])
      app.scv = Properties::RESOLVE(*this, node[STR_VERSION].Scalar());
    if (module && node[STR_ARG]) {
      if (node[STR_ARG][STR_INIT])
        app.modInit(node[STR_ARG][STR_INIT]);
      if (node[STR_ARG][STR_COMPILE])
        app.modCompile(node[STR_ARG][STR_COMPILE]);
      if (node[STR_ARG][STR_LINK])
        app.modLink(node[STR_ARG][STR_LINK]);
      if (node[STR_ARG][STR_PACK])
        app.modPack(node[STR_ARG][STR_PACK]);
      app.isMod = 1;
    }
  };
  std::vector<std::pair<std::string, std::string>> apps;
  auto lam = [&](const auto& depOrMod) {
    const kul::Dir& projectDir = resolveDepOrModDirectory(depOrMod, module);
    bool f = false;
    for (const Application* ap : vec)
      if (projectDir == ap->project().dir() && p == ap->p) {
        f = true;
        break;
      }
    if (f)
      return;
    const maiken::Project& c(
      *maiken::Projects::INSTANCE().getOrCreate(projectDir));

    auto withoutThis = [=](const std::string& name, const std::string& pro){
      for(const auto& wo : AppVars::INSTANCE().withoutParsed()){
        kul::hash::set::String profiles;
        std::string proName(wo);
        auto lb(wo.find("["));
        auto rb(wo.find("]"));
        if(lb != std::string::npos){
          if(rb == std::string::npos || rb < lb)
            KEXIT(1, "Invalid -t argument format provided");
          std::string profile = proName.substr(lb + 1, rb - lb - 1);
          for(const auto& pr : kul::String::SPLIT(profile, ",")) profiles.insert(pr);
          proName = proName.substr(0, lb);
        }
        if(name == proName){
          if(pro.empty() && profiles.count("@")) return true;
          if(profiles.count(pro)) return true;
        }
      } 
      return false;
    };

    if (depOrMod[STR_PROFILE]) {
      for (auto p : kul::String::SPLIT(
             Properties::RESOLVE(*this, depOrMod[STR_PROFILE].Scalar()), ' ')) {
        if (p.empty())
          continue;
        f = 0;
        if (p == "@")
          p = "";
        else
          for (const auto& node : c.root()[STR_PROFILE])
            if (node[STR_NAME].Scalar() == p) {
              f = 1;
              break;
            }

        if (!f && !p.empty())
          KEXIT(1,
                "profile does not exist\n" + p + "\n" + project().dir().path());

        if(!module && withoutThis(depOrMod[STR_NAME].Scalar(), p)) continue;
        auto* app = Applications::INSTANCE().getOrCreate(c, p, 0);
        if (!with)
          app->par = this;
        setApp(*app, depOrMod);
        vec.push_back(app);
        app->addRDep(this);
        apps.push_back(std::make_pair(app->project().dir().path(), app->p));
      }
    } else {
      if(!module && withoutThis(depOrMod[STR_NAME].Scalar(), "")) return;
      auto* app = Applications::INSTANCE().getOrCreate(c, "", 0);
      if (!with)
        app->par = this;
      setApp(*app, depOrMod);
      vec.push_back(app);
      app->addRDep(this);
      apps.push_back(std::make_pair(app->project().dir().path(), app->p));
    }
  };

  for (const auto& depOrMod : n[s])
    lam(depOrMod);

  if (!module && n[STR_SELF])
    for (const auto& s : kul::String::SPLIT(
           Properties::RESOLVE(*this, n[STR_SELF].Scalar()), ' ')) {
      auto* app = Applications::INSTANCE().getOrCreate(project(), s);
      app->par = this;
      app->scr = scr;
      vec.push_back(app);
      app->addRDep(this);
      apps.push_back(std::make_pair(app->project().dir().path(), app->p));
    }
  cyclicCheck(apps);
  for (auto* ap : vec) {
    auto& app(*ap);
    if (app.buildDir().path().empty()) {
      kul::env::CWD(app.project().dir());
      if (app.project().root()[STR_SCM])
        app.scr =
          Properties::RESOLVE(app, app.project().root()[STR_SCM].Scalar());
      app.setup();
      if (app.sources().size())
        app.buildDir().mk();
      kul::env::CWD(this->project().dir());
    }
    std::string _path(app.inst ? app.inst.escr() : app.buildDir().escr());
    if (app.sources().size() &&
        std::find(app.paths.begin(), app.paths.end(), _path) == app.paths.end())
      app.paths.push_back(_path);
  }
}