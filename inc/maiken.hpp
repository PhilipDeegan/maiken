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
#ifndef _MAIKEN_APP_HPP_
#define _MAIKEN_APP_HPP_

#include "maiken/defs.hpp"

#include "kul/cli.hpp"
#include "kul/log.hpp"
#include "kul/os.hpp"
#include "kul/proc.hpp"
#include "kul/scm/man.hpp"
#include "kul/threads.hpp"

#include "maiken/compiler.hpp"
#include "maiken/compiler/compilers.hpp"
#include "maiken/project.hpp"
#include "maiken/settings.hpp"
#include "maiken/global.hpp"

int
main(int argc, char* argv[]);

namespace maiken {

class Exception : public kul::Exception
{
public:
  Exception(const char* f, const uint16_t& l, const std::string& s)
    : kul::Exception(f, l, s)
  {}
};

class Module;
class ModuleLoader;
class ThreadingCompiler;
class Applications;
class KUL_PUBLISH Application : public Constants
{
  friend class Applications;
  friend class ThreadingCompiler;

protected:
  bool ig = 1, isMod = 0, ro = 0;
  const Application* par = nullptr;
  Application* sup = nullptr;
  compiler::Mode m;
  std::string arg, main, lang, lnk, out, scr, scv;
  const std::string p;
  kul::Dir bd, inst;
  YAML::Node modIArg, modCArg, modLArg, modPArg;
  const maiken::Project& proj;
  kul::hash::map::S2T<kul::hash::map::S2S> fs;
  kul::hash::map::S2S includeStamps, itss, ps;
  kul::hash::map::S2S cArg, cLnk;
  kul::hash::map::S2T<kul::hash::set::String> args;
  kul::hash::map::S2T<uint64_t> stss;
  std::vector<Application*> deps, modDeps, rdeps;
  std::vector<std::shared_ptr<ModuleLoader>> mods;
  std::vector<kul::cli::EnvVar> evs;
  std::vector<std::string> libs, paths;
  std::vector<std::pair<std::string, bool>> incs, srcs;
  const kul::SCM* scm = 0;

  CompilerProcessCapture buildExecutable(const kul::hash::set::String& objects);
  CompilerProcessCapture buildLibrary(const kul::hash::set::String& objects);
  void checkErrors(const CompilerProcessCapture& cpc) KTHROW(kul::Exception);

  void populateMaps(const YAML::Node& n) KTHROW(kul::Exception);

  void preSetupValidation() KTHROW(Exception);
  void postSetupValidation() KTHROW(Exception);
  void resolveProperties() KTHROW(Exception);
  void resolveLang() KTHROW(Exception);
  static void parseDependencyString(std::string s, kul::hash::set::String& include);

  void compile(kul::hash::set::String& objects) KTHROW(kul::Exception);
  void build() KTHROW(kul::Exception);
  void pack() KTHROW(kul::Exception);
  void findObjects(kul::hash::set::String& objects) const;
  void link(const kul::hash::set::String& objects) KTHROW(kul::Exception);
  void run(bool dbg);
  void trim();
  void trim(const kul::File& f);

  void scmStatus(const bool& deps = false) KTHROW(kul::scm::Exception);
  void scmUpdate(const bool& f) KTHROW(kul::scm::Exception);
  void scmUpdate(const bool& f, const kul::SCM* scm, const std::string& repo)
    KTHROW(kul::scm::Exception);

  void setup() KTHROW(kul::Exception);
  void setSuper();
  void showConfig(bool force = 0);
  void showTree() const;
  void showTreeRecursive(uint8_t i) const;
  void cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps)
    const KTHROW(kul::Exception);
  void showProfiles();

  void writeTimeStamps(kul::hash::set::String& objects,
                       std::vector<kul::File>& cacheFiles);
  void loadTimeStamps() KTHROW(kul::StringException);

  void buildDepVec(const std::string* depVal);
  void buildDepVecRec(
    std::unordered_map<uint16_t, std::vector<Application*>>& dePs,
    int16_t ig,
    int16_t i,
    const kul::hash::set::String& inc);

  void populateMapsFromDependencies() KTHROW(kul::Exception);

  void loadDepOrMod(const YAML::Node& node,
                    const kul::Dir& depOrMod,
                    bool module) KTHROW(kul::Exception);
  kul::Dir resolveDepOrModDirectory(const YAML::Node& d, bool module);
  void popDepOrMod(const YAML::Node& n,
                   std::vector<Application*>& vec,
                   const std::string& s,
                   bool module,
                   bool with = 0) KTHROW(kul::Exception);

  kul::hash::set::String inactiveMains() const;

  bool incSrc(const kul::File& f) const;
  void addCLIArgs(const kul::cli::Args& args);
  void withArgs(std::vector<YAML::Node>& with_nodes, std::function<void(const YAML::Node& n, const bool mod)> getIfMissing);
  void addSourceLine(const std::string& o) KTHROW(kul::Exception);
  void addIncludeLine(const std::string& o) KTHROW(kul::Exception);

  void modInit(const YAML::Node& modArg) { modIArg = modArg; }
  const YAML::Node& modInit() { return modIArg; }
  void modCompile(const YAML::Node& modArg) { modCArg = modArg; }
  const YAML::Node& modCompile() { return modCArg; }
  void modLink(const YAML::Node& modArg) { modLArg = modArg; }
  const YAML::Node& modLink() { return modLArg; }
  void modPack(const YAML::Node& modArg) { modPArg = modArg; }
  const YAML::Node& modPack() { return modPArg; }

  void addRDep(Application* app){
    if(std::find(rdeps.begin(), rdeps.end(), app) == rdeps.end())
      rdeps.push_back(app);
  }

  static void showHelp();

public:
  Application(const maiken::Project& proj,
              const std::string& profile =
                ""); // : m(Mode::NONE), p(profile), proj(proj){}
  Application(const Application& a) = delete;
  Application(const Application&& a) = delete;
  Application& operator=(const Application& a) = delete;
  ~Application();

  virtual void process() KTHROW(kul::Exception);
  const kul::Dir& buildDir() const { return bd; }
  const std::string& profile() const { return p; }
  const maiken::Project& project() const { return proj; }
  const std::vector<Application*>& dependencies() const { return deps; }
  const std::vector<Application*>& revendencies() const { return rdeps; }
  const std::vector<Application*>& moduleDependencies() const
  {
    return modDeps;
  }
  const std::vector<std::shared_ptr<ModuleLoader>>& modules() const
  {
    return mods;
  }
  const std::vector<kul::cli::EnvVar>& envVars() const { return evs; }
  const kul::hash::map::S2T<kul::hash::map::S2S>& files() const { return fs; }
  const std::vector<std::string>& libraries() const { return libs; }
  const std::vector<std::pair<std::string, bool>>& sources() const
  {
    return srcs;
  }
  const std::vector<std::pair<std::string, bool>>& includes() const
  {
    return incs;
  }
  const std::vector<std::string>& libraryPaths() const { return paths; }
  const kul::hash::map::S2S& properties() const { return ps; }
  const kul::hash::map::S2T<kul::hash::set::String>& arguments() const
  {
    return args;
  }

  void addInclude(const std::string &s, bool p = 1){ incs.push_back(std::make_pair(s, p)); }
  void addLibpath(const std::string &s){ paths.push_back(s); }

  std::string baseLibFilename() const
  {
    std::string n = project().root()[STR_NAME].Scalar();
    return out.empty() ? inst ? p.empty() ? n : n + "_" + p : n : out;
  }
  kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> sourceMap()
    const;

  static std::vector<Application*> CREATE(int16_t argc, char* argv[])
    KTHROW(kul::Exception);
};

class Applications : public Constants
{
  friend int ::main(int argc, char* argv[]);

private:
  kul::hash::map::S2T<kul::hash::map::S2T<Application*>> m_apps;
  std::vector<std::unique_ptr<Application>> m_appPs;
  Applications() {}
  void clear()
  {
    m_apps.clear();
    m_appPs.clear();
  }

public:
  static Applications& INSTANCE()
  {
    static Applications a;
    return a;
  }
  Application* getOrCreate(const maiken::Project& proj,
                           const std::string& _profile = "",
                           bool setup = 1) KTHROW(kul::Exception)
  {
    std::string pDir(proj.dir().real());
    std::string profile = _profile.empty() ? "@" : _profile;
    if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
      auto app = std::make_unique<Application>(proj, _profile);
      auto pp = app.get();
      m_appPs.push_back(std::move(app));
      m_apps[pDir][profile] = pp;
      if (setup) {
        const std::string& cwd(kul::env::CWD());
        kul::env::CWD(proj.dir());
        pp->setup();
        kul::env::CWD(cwd);
      }
    }
    return m_apps[pDir][profile];
  }
  Application* getOrCreateRoot(const maiken::Project& proj,
                               const std::string& _profile = "",
                               bool setup = 1) KTHROW(kul::Exception)
  {
    std::string pDir(proj.dir().real());
    std::string profile = _profile.empty() ? "@" : _profile;
    if (!m_apps.count(pDir) || !m_apps[pDir].count(profile)) {
      auto app = std::make_unique<Application>(proj, _profile);
      auto pp = app.get();
      pp->ro = 1;
      m_appPs.push_back(std::move(app));
      m_apps[pDir][profile] = pp;
      if (setup) {
        const std::string& cwd(kul::env::CWD());
        kul::env::CWD(proj.dir());
        pp->setup();
        kul::env::CWD(cwd);
      }
    }
    return m_apps[pDir][profile];
  }
  Application* getOrNullptr(const std::string& project)
  {
    uint8_t count = 0;
    Application* app = nullptr;
    for (const auto p1 : m_apps)
      for (const auto p2 : p1.second) {
        if (p2.second->project().root()[STR_NAME].Scalar() == project) {
          count++;
          app = p2.second;
        }
      }
    if (count > 1) {
      KEXIT(1, "Cannot deduce project version as")
        << " there are multiple versions in the dependency tree";
    }
    return app;
  }
};

class ThreadingCompiler : public Constants
{
private:
  maiken::Application& app;
  std::vector<std::string> incs;

public:
  ThreadingCompiler(maiken::Application& app)
    : app(app)
  {
    for (const auto& s : app.includes()) {
      kul::Dir d(s.first);
      const std::string& m(AppVars::INSTANCE().dryRun() ? d.esc() : d.escm());
      if (!m.empty())
        incs.push_back(m);
      else
        incs.push_back(".");
    }
  }
  CompilerProcessCapture compile(
    const std::pair<std::string, std::string>& pair) const
    KTHROW(kul::Exception);
};

class SCMGetter
{
private:
  kul::hash::map::S2S valids;
  static bool IS_SOLID(const std::string& r)
  {
    return r.find("://") != std::string::npos ||
           r.find("@") != std::string::npos;
  }
  static SCMGetter& INSTANCE()
  {
    static SCMGetter s;
    return s;
  }
  static const kul::SCM* GET_SCM(const kul::Dir& d,
                                 const std::string& r,
                                 bool module);

public:
  static const std::string REPO(const kul::Dir& d,
                                const std::string& r,
                                bool module)
  {
    if (INSTANCE().valids.count(d.path()))
      return (*INSTANCE().valids.find(d.path())).second;
    if (IS_SOLID(r))
      INSTANCE().valids.insert(d.path(), r);
    else
      GET_SCM(d, r, module);
    if (INSTANCE().valids.count(d.path()))
      return (*INSTANCE().valids.find(d.path())).second;
    KEXCEPT(Exception, "SCM not discovered for project: " + d.path());
  }
  static bool HAS(const kul::Dir& d)
  {
    return (kul::Dir(d.join(".git")) || kul::Dir(d.join(".svn")));
  }
  static const kul::SCM* GET(const kul::Dir& d,
                             const std::string& r,
                             bool module)
  {
    if (IS_SOLID(r))
      INSTANCE().valids.insert(d.path(), r);
    if (kul::Dir(d.join(".git")))
      return &kul::scm::Manager::INSTANCE().get("git");
    if (kul::Dir(d.join(".svn")))
      return &kul::scm::Manager::INSTANCE().get("svn");
    return r.size() ? GET_SCM(d, r, module) : 0;
  }
};

} // namespace maiken
#endif /* _MAIKEN_APP_HPP_ */

#include <maiken/module.hpp>
#include <maiken/property.hpp>
