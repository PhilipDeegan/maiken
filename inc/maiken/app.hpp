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
#include "maiken/except.hpp"
#include "maiken/global.hpp"
#include "maiken/project.hpp"

int main(int argc, char *argv[]);

namespace maiken {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
namespace dist {
class CompileRequest;
}  //  end namespace dist
#endif  //  _MKN_WITH_MKN_RAM_  &&         _MKN_WITH_IO_CEREAL_

static std::string PROGRAM = "";

class Module;
class ModuleLoader;
class ThreadingCompiler;
class Applications;
class SourceFinder;
class CompilerPrinter;
class KUL_PUBLISH Application : public Constants {
  friend class Applications;
  friend class CompilerPrinter;
  friend class Executioner;
  friend class SourceFinder;
  friend class ThreadingCompiler;
  friend class Project;
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  friend class dist::CompileRequest;
#endif  //  _MKN_WITH_MKN_RAM_  &&         _MKN_WITH_IO_CEREAL_

 protected:
  bool ig = 1, isMod = 0, ro = 0;
  const Application *par = nullptr;
  Application *sup = nullptr;
  compiler::Mode m;
  std::string arg, bin, lang, lnk, main, out, scr, scv;
  const std::string p;
  kul::Dir bd, inst;
  std::unordered_map<const Application *, YAML::Node> modIArgs, modCArgs, modLArgs, modPArgs;
  const maiken::Project &proj;
  kul::hash::map::S2T<kul::hash::map::S2S> fs;
  kul::hash::map::S2S cArg, cLnk, includeStamps, itss, ps, tests;
  kul::hash::map::S2T<kul::hash::set::String> args;
  kul::hash::map::S2T<uint64_t> stss;
  std::vector<Application *> deps, modDeps, rdeps;
  std::vector<std::shared_ptr<ModuleLoader>> mods;
  std::vector<kul::cli::EnvVar> evs;
  std::vector<std::string> defs, libs, paths;
  std::vector<std::pair<std::string, bool>> incs, srcs;
  const kul::SCM *scm = 0;

  void buildExecutable(const kul::hash::set::String &objects) KTHROW(kul::Exception);
  CompilerProcessCapture buildLibrary(const kul::hash::set::String &objects) KTHROW(kul::Exception);
  void buildTest(const kul::hash::set::String &objects) KTHROW(kul::Exception);
  void checkErrors(const CompilerProcessCapture &cpc) KTHROW(kul::Exception);

  void populateMaps(const YAML::Node &n) KTHROW(kul::Exception);

  void preSetupValidation() KTHROW(Exception);
  void postSetupValidation() KTHROW(Exception);
  void resolveProperties() KTHROW(Exception);
  void resolveLang() KTHROW(Exception);
  static void parseDependencyString(std::string s, kul::hash::set::String &include);

  void compile(kul::hash::set::String &objects) KTHROW(kul::Exception);
  void compile(std::vector<std::pair<std::string, std::string>> &src_objs,
               kul::hash::set::String &objects, std::vector<kul::File> &cacheFiles)
      KTHROW(kul::Exception);
  void compile(std::queue<std::pair<std::string, std::string>> &src_objs,
               kul::hash::set::String &objects, std::vector<kul::File> &cacheFiles)
      KTHROW(kul::Exception);
  void build() KTHROW(kul::Exception);
  void pack() KTHROW(kul::Exception);
  void findObjects(kul::hash::set::String &objects) const;
  void link(const kul::hash::set::String &objects) KTHROW(kul::Exception);
  void run(bool dbg);
  void test();

  void scmStatus(const bool &deps = false) KTHROW(kul::scm::Exception);
  void scmUpdate(const bool &f) KTHROW(kul::scm::Exception);
  void scmUpdate(const bool &f, const kul::SCM *scm, const std::string &repo)
      KTHROW(kul::scm::Exception);

  void setup() KTHROW(kul::Exception);
  void setSuper();
  void showConfig(bool force = 0);
  void showTree() const;
  void showTreeRecursive(uint8_t i) const;
  void cyclicCheck(const std::vector<std::pair<std::string, std::string>> &apps) const
      KTHROW(kul::Exception);
  void showProfiles();

  void writeTimeStamps(kul::hash::set::String &objects, std::vector<kul::File> &cacheFiles);
  void loadTimeStamps() KTHROW(kul::StringException);

  void buildDepVec(const std::string &depVal);
  void buildDepVecRec(std::unordered_map<uint16_t, std::vector<Application *>> &dePs, int16_t ig,
                      int16_t i, const kul::hash::set::String &inc);

  void populateMapsFromDependencies() KTHROW(kul::Exception);

  void loadDepOrMod(const YAML::Node &node, const kul::Dir &depOrMod, bool module)
      KTHROW(kul::Exception);
  kul::Dir resolveDepOrModDirectory(const YAML::Node &d, bool module);
  void popDepOrMod(const YAML::Node &n, std::vector<Application *> &vec, const std::string &s,
                   bool module, bool with = 0) KTHROW(kul::Exception);

  kul::hash::set::String inactiveMains() const;

  bool incSrc(const kul::File &f) const;
  void addCLIArgs(const kul::cli::Args &args);
  void addSourceLine(const std::string &o) KTHROW(kul::Exception);
  void addIncludeLine(const std::string &o) KTHROW(kul::Exception);

  void modInit(const Application *const other, const YAML::Node &modArg) {
    modIArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modInit(const Application *const other) const {
    if (modIArgs.count(other)) return (*modIArgs.find(other)).second;
    return YAML::Node();
  }

  void modCompile(const Application *const other, const YAML::Node &modArg) {
    modCArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modCompile(const Application *const other) const {
    if (modCArgs.count(other)) return (*modCArgs.find(other)).second;
    return YAML::Node();
  }

  void modLink(const Application *const other, const YAML::Node &modArg) {
    modLArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modLink(const Application *const other) const {
    if (modLArgs.count(other)) return (*modLArgs.find(other)).second;
    return YAML::Node();
  }

  void modPack(const Application *const other, const YAML::Node &modArg) {
    modPArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modPack(const Application *const other) const {
    if (modPArgs.count(other)) return (*modPArgs.find(other)).second;
    return YAML::Node();
  }

  void addRDep(Application *app) {
    if (std::find(rdeps.begin(), rdeps.end(), app) == rdeps.end()) rdeps.push_back(app);
  }

  bool is_build_required();
  bool is_build_stale();

  void withArgs(const std::string with, std::vector<YAML::Node> &with_nodes,
                std::function<void(const YAML::Node &n, const bool mod)> getIfMissing, bool root,
                bool dep);
  void with(kul::hash::set::String &withs, std::vector<YAML::Node> &with_nodes,
            std::function<void(const YAML::Node &n, const bool mod)> getIfMissing, bool dep);

  void modArgs(const std::string mod_str, std::vector<YAML::Node> &mod_nodes,
               std::function<void(const YAML::Node &n, const bool mod)> getIfMissing);
  void mod(kul::hash::set::String &mods, std::vector<YAML::Node> &mod_nodes,
           std::function<void(const YAML::Node &n, const bool mod)> getIfMissing);

  static void showHelp();

 public:
  Application(const maiken::Project &proj, const std::string &profile = "");
  Application(const Application &a) = delete;
  Application(const Application &&a) = delete;
  Application &operator=(const Application &a) = delete;
  virtual ~Application();

  std::string getMain() { return main; }
  virtual void process() KTHROW(kul::Exception);
  const kul::Dir &buildDir() const { return bd; }
  const std::string &binary() const { return bin; }
  const std::string &profile() const { return p; }
  const maiken::Project &project() const { return proj; }
  const std::vector<Application *> &dependencies() const { return deps; }
  const std::vector<Application *> &revendencies() const { return rdeps; }
  const std::vector<Application *> &moduleDependencies() const { return modDeps; }
  const std::vector<std::shared_ptr<ModuleLoader>> &modules() const { return mods; }
  const kul::hash::map::S2T<kul::hash::map::S2S> &files() const { return fs; }
  const std::vector<std::string> &libraries() const { return libs; }
  const std::vector<std::pair<std::string, bool>> &sources() const { return srcs; }
  const std::vector<std::pair<std::string, bool>> &includes() const { return incs; }
  const std::vector<std::string> &libraryPaths() const { return paths; }
  const kul::hash::map::S2S &properties() const { return ps; }
  const kul::hash::map::S2T<kul::hash::set::String> &arguments() const { return args; }

  std::vector<kul::cli::EnvVar> &envVars() { return evs; }

  void add_def(const std::string &def) { defs.emplace_back(def); }
  const std::vector<std::string> &defines() const { return defs; }

#ifdef _MKN_WITH_MKN_RAM_
  bool get_binaries();
#endif

  void addInclude(const std::string &s, bool p = 1) {
    auto it = std::find_if(
        incs.begin(), incs.end(),
        [&](const std::pair<std::string, bool> &element) { return element.first == s; });
    if (it == incs.end()) incs.push_back(std::make_pair(s, p));
  }
  void addLibpath(const std::string &s) { paths.push_back(s); }

  void prependCompileString(const std::string &s) { arg = s + " " + arg; }
  void prependLinkString(const std::string &s) { lnk = s + " " + lnk; }

  std::string baseLibFilename() const {
    std::string n = project().root()[STR_NAME].Scalar();
    return out.empty() ? inst ? p.empty() ? n : n + "_" + p : n : out;
  }
  kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> sourceMap() const;

  static std::vector<Application *> CREATE(const kul::cli::Args &args) KTHROW(kul::Exception);
  static std::vector<Application *> CREATE(int16_t argc, char *argv[]) KTHROW(kul::Exception);
};

class Applications : public Constants {
  friend int ::main(int argc, char *argv[]);

 public:
  static Applications &INSTANCE() {
    static Applications a;
    return a;
  }
  Application *getOrCreate(const maiken::Project &proj, const std::string &_profile = "",
                           bool setup = 1) KTHROW(kul::Exception);

  Application *getOrCreateRoot(const maiken::Project &proj, const std::string &_profile = "",
                               bool setup = 1) KTHROW(kul::Exception);

  Application *getOrNullptr(const std::string &project);

  std::vector<Application const *> applicationsFor(Project const &project) const {
    std::vector<Application const *> ret;
    for (auto const &profile : m_apps.at(project.dir().real())) ret.emplace_back(profile.second);
    return ret;
  }

 private:
  kul::hash::map::S2T<kul::hash::map::S2T<Application *>> m_apps;
  std::vector<std::unique_ptr<Application>> m_appPs;
  Applications() {}
  void clear() {
    m_apps.clear();
    m_appPs.clear();
  }
};

class ThreadingCompiler : public Constants {
 private:
  maiken::Application &app;
  std::vector<std::string> incs;

 public:
  ThreadingCompiler(maiken::Application &app) : app(app) {
    for (const auto &s : app.includes()) {
      std::string m;
      kul::Dir d(s.first);
      kul::File f(s.first);
      if (d)
        m = (AppVars::INSTANCE().dryRun() ? d.esc() : d.escm());
      else if (f)
        m = (AppVars::INSTANCE().dryRun() ? f.esc() : f.escm());
      if (!m.empty())
        incs.push_back(m);
      else
        incs.push_back(".");
    }
  }

  CompilationUnit compilationUnit(const std::pair<std::string, std::string> &pair) const
      KTHROW(kul::Exception);
};

class ModuleMinimiser {
  friend class maiken::Application;

 private:
  static void add(const std::vector<maiken::Application *> &mods,
                  kul::hash::map::S2T<maiken::Application *> &apps) {
    for (auto *const m : mods)
      if (!apps.count(m->buildDir().real())) apps.insert(m->buildDir().real(), m);
  }

 public:
  static kul::hash::map::S2T<maiken::Application *> modules(maiken::Application &app) {
    kul::hash::map::S2T<maiken::Application *> apps;
    add(app.moduleDependencies(), apps);
    for (auto dep = app.dependencies().rbegin(); dep != app.dependencies().rend(); ++dep)
      add((*dep)->moduleDependencies(), apps);
    return apps;
  }
};

class CommandStateMachine {
  friend class maiken::Application;

 private:
  bool _main = 1;
  kul::hash::set::String cmds;
  CommandStateMachine() { reset(); }
  static CommandStateMachine &INSTANCE() {
    static CommandStateMachine a;
    return a;
  }
  void reset() {
    cmds.clear();
    for (const auto &s : maiken::AppVars::INSTANCE().commands()) cmds.insert(s);
  }
  void add(const std::string &s) { cmds.insert(s); }
  const kul::hash::set::String &commands() const { return cmds; }
  void main(bool m) { _main = m; }
  bool main() const { return _main; }
};

class BuildRecorder {
  friend class maiken::Application;

 private:
  kul::hash::set::String builds;
  static BuildRecorder &INSTANCE() {
    static BuildRecorder a;
    return a;
  }
  void add(const std::string &k) { builds.insert(k); }
  bool has(const std::string &k) { return builds.count(k); }
};

class CompilerValidation : public Constants {
 public:
  static void check_compiler_for(
      const maiken::Application &app,
      const kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> &sources);
};
}  // namespace maiken
#endif /* _MAIKEN_APP_HPP_ */
