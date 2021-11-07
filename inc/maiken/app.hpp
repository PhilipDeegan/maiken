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

namespace maiken {
class Application;
}

#include <optional>

#include "maiken/defs.hpp"

#include "mkn/kul/cli.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/scm/man.hpp"
#include "mkn/kul/threads.hpp"

#include "maiken/compiler.hpp"
#include "maiken/compiler/compilers.hpp"
#include "maiken/except.hpp"
#include "maiken/global.hpp"
#include "maiken/project.hpp"
#include "maiken/string.hpp"
#include "maiken/source.hpp"

int main(int argc, char* argv[]);

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
class Source;
class CompilerPrinter;
class Processor;
class KUL_PUBLISH Application : public Constants {
  using This = Application;
  friend class Applications;
  friend class CompilerPrinter;
  friend class Executioner;
  friend class SourceFinder;
  friend class ThreadingCompiler;
  friend class Project;
  friend class Processor;

 public:
  using SourceMap = mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<std::vector<maiken::Source>>>;

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  friend class dist::CompileRequest;
#endif  //  _MKN_WITH_MKN_RAM_  &&         _MKN_WITH_IO_CEREAL_

 protected:
  void buildExecutable(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception);
  CompilerProcessCapture buildLibrary(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception);
  void buildTest(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception);
  void checkErrors(CompilerProcessCapture const& cpc) KTHROW(mkn::kul::Exception);

  void populateMaps(YAML::Node const& n) KTHROW(mkn::kul::Exception);

  void preSetupValidation() KTHROW(Exception);
  void postSetupValidation() KTHROW(Exception);
  void resolveProperties() KTHROW(Exception);
  void resolveLang() KTHROW(Exception);
  static void parseDependencyString(std::string s, mkn::kul::hash::set::String& include);

  void compile(mkn::kul::hash::set::String& objects) KTHROW(mkn::kul::Exception);
  void compile(std::vector<std::pair<maiken::Source, std::string>>& src_objs,
               mkn::kul::hash::set::String& objects, std::vector<mkn::kul::File>& cacheFiles)
      KTHROW(mkn::kul::Exception);
  void compile(std::queue<std::pair<maiken::Source, std::string>>& src_objs,
               mkn::kul::hash::set::String& objects, std::vector<mkn::kul::File>& cacheFiles)
      KTHROW(mkn::kul::Exception);
  void build() KTHROW(mkn::kul::Exception);
  void pack() KTHROW(mkn::kul::Exception);
  void findObjects(mkn::kul::hash::set::String& objects) const;
  void link(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception);
  void run(bool dbg);
  void test();

  void scmStatus(bool const& deps = false) KTHROW(mkn::kul::scm::Exception);
  void scmUpdate(bool const& f) KTHROW(mkn::kul::scm::Exception);
  void scmUpdate(bool const& f, const mkn::kul::SCM* scm, std::string const& repo)
      KTHROW(mkn::kul::scm::Exception);

  void setup() KTHROW(mkn::kul::Exception);
  void setSuper();
  void showConfig(bool force = 0);
  void showTree() const;
  void showTreeRecursive(uint8_t i) const;
  void cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const
      KTHROW(mkn::kul::Exception);
  void showProfiles();

  void writeTimeStamps(mkn::kul::hash::set::String& objects, std::vector<mkn::kul::File>& cacheFiles);
  void loadTimeStamps() KTHROW(mkn::kul::StringException);

  void buildDepVec(std::string const& depVal);
  void buildDepVecRec(std::unordered_map<uint16_t, std::vector<Application*>>& dePs, int16_t ig,
                      int16_t i, mkn::kul::hash::set::String const& inc);

  void populateMapsFromDependencies() KTHROW(mkn::kul::Exception);

  void loadDepOrMod(YAML::Node const& node, const mkn::kul::Dir& depOrMod, bool module)
      KTHROW(mkn::kul::Exception);
  mkn::kul::Dir resolveDepOrModDirectory(YAML::Node const& d, bool module);
  void popDepOrMod(YAML::Node const& n, std::vector<Application*>& vec, std::string const& s,
                   bool module, bool with = 0) KTHROW(mkn::kul::Exception);

  mkn::kul::hash::set::String inactiveMains() const;

  bool incSrc(mkn::kul::File const& f) const;
  void addCLIArgs(mkn::kul::cli::Args const& args);

  void addSourceLine(std::string const& o) KTHROW(mkn::kul::Exception);
  void addMainLine(std::string const& o) KTHROW(mkn::kul::Exception);

  void addIncludeLine(std::string const& o) KTHROW(mkn::kul::Exception);

  void modInit(Application const* const other, YAML::Node const& modArg) {
    modIArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modInit(Application const* const other) const {
    if (modIArgs.count(other)) return (*modIArgs.find(other)).second;
    return YAML::Node();
  }

  void modCompile(Application const* const other, YAML::Node const& modArg) {
    modCArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modCompile(Application const* const other) const {
    if (modCArgs.count(other)) return (*modCArgs.find(other)).second;
    return YAML::Node();
  }

  void modLink(Application const* const other, YAML::Node const& modArg) {
    modLArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modLink(Application const* const other) const {
    if (modLArgs.count(other)) return (*modLArgs.find(other)).second;
    return YAML::Node();
  }

  void modTest(Application const* const other, YAML::Node const& modArg) {
    modTArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modTest(Application const* const other) const {
    if (modTArgs.count(other)) return (*modTArgs.find(other)).second;
    return YAML::Node();
  }

  void modPack(Application const* const other, YAML::Node const& modArg) {
    modPArgs.emplace(std::make_pair(other, modArg));
  }
  YAML::Node modPack(Application const* const other) const {
    if (modPArgs.count(other)) return (*modPArgs.find(other)).second;
    return YAML::Node();
  }

  void addRDep(Application* app) {
    if (std::find(rdeps.begin(), rdeps.end(), app) == rdeps.end()) rdeps.push_back(app);
  }

  bool is_build_required();
  bool is_build_stale();

  void withArgs(std::string const with, std::vector<YAML::Node>& with_nodes,
                std::function<void(YAML::Node const& n, bool const mod)> getIfMissing, bool dep);
  void with(mkn::kul::hash::set::String& withs, std::vector<YAML::Node>& with_nodes,
            std::function<void(YAML::Node const& n, bool const mod)> getIfMissing, bool dep);

  void modArgs(std::string const mod_str, std::vector<YAML::Node>& mod_nodes,
               std::function<void(YAML::Node const& n, bool const mod)> getIfMissing);
  void mod(mkn::kul::hash::set::String& mods, std::vector<YAML::Node>& mod_nodes,
           std::function<void(YAML::Node const& n, bool const mod)> getIfMissing);

  static void showHelp();

 public:
  Application(maiken::Project const& proj, std::string const& profile = "");
  Application(Application const& a) = delete;
  Application(Application const&& a) = delete;
  Application& operator=(Application const& a) = delete;
  virtual ~Application();

  auto& main() const { return main_; }
  virtual void process() KTHROW(mkn::kul::Exception);
  const mkn::kul::Dir& buildDir() const { return bd; }
  std::string const& binary() const { return bin; }
  std::string const& profile() const { return p; }
  maiken::Project const& project() const { return proj; }
  const std::vector<Application*>& dependencies() const { return deps; }
  const std::vector<Application*>& revendencies() const { return rdeps; }
  const std::vector<Application*>& moduleDependencies() const { return modDeps; }
  const std::vector<std::shared_ptr<ModuleLoader>>& modules() const { return mods; }
  const mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2S>& files() const { return fs; }
  std::vector<std::string> const& libraries() const { return libs; }
  const std::vector<std::pair<Source, bool>>& sources() const { return srcs; }
  const std::vector<std::pair<std::string, bool>>& includes() const { return incs; }
  std::vector<std::string> const& libraryPaths() const { return paths; }
  const mkn::kul::hash::map::S2S& properties() const { return ps; }
  const mkn::kul::hash::map::S2T<mkn::kul::hash::set::String>& arguments() const { return args; }

  auto& mode() const { return m; }
  void mode(compiler::Mode mode) { m = mode; }

  std::vector<mkn::kul::cli::EnvVar>& envVars() { return evs; }
  const std::vector<mkn::kul::cli::EnvVar>& envVars() const { return evs; }

  void add_def(std::string const& def) { defs.emplace_back(def); }
  std::vector<std::string> const& defines() const { return defs; }

  void addInclude(std::string const& s, bool p = 1) {
    auto it = std::find_if(
        incs.begin(), incs.end(),
        [&](const std::pair<std::string, bool>& element) { return element.first == s; });
    if (it == incs.end()) incs.emplace_back(s, p);
  }
  void addLibpath(std::string const& s) { paths.push_back(s); }

  void prependCompileString(std::string const& s) { arg = s + " " + arg; }
  void prependLinkString(std::string const& s) { lnk = s + " " + lnk; }

  auto root() const { return ro; }

  std::string baseLibFilename() const {
    std::string n = project().root()[STR_NAME].Scalar();
    return out.empty() ? inst ? p.empty() ? n : n + "_" + p : n : out;
  }
  SourceMap sourceMap() const;

  static std::vector<Application*> CREATE(mkn::kul::cli::Args const& args) KTHROW(mkn::kul::Exception);
  static std::vector<Application*> CREATE(int16_t argc, char* argv[]) KTHROW(mkn::kul::Exception);

  static mkn::kul::cli::EnvVar PARSE_ENV_NODE(YAML::Node const&, Application const&);
  static mkn::kul::cli::EnvVar PARSE_PROFILE_NAME(YAML::Node const&, Application* = nullptr);

  static std::string hash(std::string in) {
    std::stringstream ss;
    ss << std::hex << std::hash<std::string>()(in);
    return ss.str();
  }

  std::string hash() { return hash(project().dir().real()) + "_" + hash(p.empty() ? "@" : p); }

  CompilationInfo m_cInfo;

 protected:
  bool ig = 1, isMod = 0, ro = 0;
  Application const* par = nullptr;
  Application* sup = nullptr;
  compiler::Mode m{compiler::Mode::SHAR};
  std::string arg, bin, lang, lnk, out, scr, scv;
  std::optional<Source> main_;
  std::string const p;
  mkn::kul::Dir bd, inst;
  std::unordered_map<const This*, YAML::Node> modIArgs, modCArgs, modLArgs, modTArgs, modPArgs;
  maiken::Project const& proj;
  mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2S> fs;
  mkn::kul::hash::map::S2S cArg, cLnk, includeStamps, itss, ps, tests;
  mkn::kul::hash::map::S2T<mkn::kul::hash::set::String> args;
  mkn::kul::hash::map::S2T<uint64_t> stss;
  std::vector<Application*> deps, modDeps, rdeps;
  std::vector<std::shared_ptr<ModuleLoader>> mods;
  std::vector<mkn::kul::cli::EnvVar> evs;
  std::vector<std::string> defs, libs, paths;
  std::vector<std::pair<maiken::Source, bool>> srcs;
  std::vector<std::pair<std::string, bool>> incs;
  const mkn::kul::SCM* scm = 0;
};

class Applications : public Constants {
  friend int ::main(int argc, char* argv[]);

 public:
  static Applications& INSTANCE() {
    static Applications a;
    return a;
  }
  Application* getOrCreate(maiken::Project const& proj, std::string const& _profile = "",
                           bool setup = 1) KTHROW(mkn::kul::Exception);

  Application* getOrCreateRoot(maiken::Project const& proj, std::string const& _profile = "",
                               bool setup = 1) KTHROW(mkn::kul::Exception);

  Application* getOrNullptr(std::string const& project);

  std::vector<Application const*> applicationsFor(Project const& project) const {
    std::vector<Application const*> ret;
    for (auto const& profile : m_apps.at(project.dir().real())) ret.emplace_back(profile.second);
    return ret;
  }

 private:
  mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<Application*>> m_apps;
  std::vector<std::unique_ptr<Application>> m_appPs;
  Applications() {}
  void clear() {
    m_apps.clear();
    m_appPs.clear();
  }
};

class ThreadingCompiler : public Constants {
 private:
  maiken::Application& app;
  std::vector<std::string> incs;

 public:
  ThreadingCompiler(maiken::Application& app) : app(app) {
    for (auto const& s : app.includes()) {
      std::string m;
      mkn::kul::Dir d(s.first);
      mkn::kul::File f(s.first);
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

  CompilationUnit compilationUnit(std::pair<maiken::Source, std::string> const& pair) const
      KTHROW(mkn::kul::Exception);
};

class ModuleMinimiser {
  friend class maiken::Application;

 private:
  static void add(std::vector<maiken::Application*> const& mods,
                  mkn::kul::hash::map::S2T<maiken::Application*>& apps) {
    for (auto* const m : mods)
      if (!apps.count(m->buildDir().real())) apps.insert(m->buildDir().real(), m);
  }

 public:
  static mkn::kul::hash::map::S2T<maiken::Application*> modules(maiken::Application& app) {
    mkn::kul::hash::map::S2T<maiken::Application*> apps;
    add(app.moduleDependencies(), apps);
    for (auto dep = app.dependencies().rbegin(); dep != app.dependencies().rend(); ++dep)
      add((*dep)->moduleDependencies(), apps);
    return apps;
  }
};

class CommandStateMachine {
  friend class maiken::Application;
  friend class maiken::Processor;

 public:
  static CommandStateMachine& INSTANCE() {
    static CommandStateMachine a;
    return a;
  }
  static bool has(std::string cmd) { return INSTANCE().cmds.count(cmd); }
  mkn::kul::hash::set::String const& commands() const { return cmds; }
  bool main() const { return _main; }

 private:
  CommandStateMachine() { reset(); }

  void main(bool m) { _main = m; }
  void add(std::string const& s) { cmds.insert(s); }

  void reset() {
    cmds.clear();
    for (auto const& s : maiken::AppVars::INSTANCE().commands()) cmds.insert(s);
  }

  bool _main = 1;
  mkn::kul::hash::set::String cmds;
};

class BuildRecorder {
  friend class maiken::Application;

 private:
  mkn::kul::hash::set::String builds;
  static BuildRecorder& INSTANCE() {
    static BuildRecorder a;
    return a;
  }
  void add(std::string const& k) { builds.insert(k); }
  bool has(std::string const& k) { return builds.count(k); }
};

class CompilerValidation : public Constants {
 public:
  static void check_compiler_for(maiken::Application const& app,
                                 Application::SourceMap const& sources);
};
}  // namespace maiken

#include "maiken/processor.hpp"
#endif /* _MAIKEN_APP_HPP_ */
