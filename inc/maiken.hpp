/**
Copyright (c) 2013, Philip Deegan.
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

#include "kul/os.hpp"
#include "kul/cli.hpp"
#include "kul/log.hpp"
#include "kul/proc.hpp"
#include "kul/scm/man.hpp"
#include "kul/threads.hpp"
#include "kul/code/compilers.hpp"

#include "maiken/defs.hpp"
#include "maiken/project.hpp"
#include "maiken/settings.hpp"

int main(int argc, char* argv[]);

namespace maiken{

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class AppVars : public Constants{
    private:
        bool d = 0, dr = 0, f = 0, s = 0, sh = 0, st = 0, t = 0, u = 0;
        uint16_t dl = 0, ts = 1;
        std::string aa, al, la, ra;
        const std::string* dep;
        kul::hash::set::String cmds, modPhases;
        kul::hash::map::S2S evs, jas, pks;
        AppVars(){
            pks["OS"]   = KTOSTRING(__KUL_OS__);
            pks["HOME"] = kul::user::home().path();
            pks["MKN_HOME"] = kul::user::home(STR_MAIKEN).path();
            pks["DATETIME"] = kul::DateTime::NOW();
            pks["TIMESTAMP"] = std::time(NULL);

            if(Settings::INSTANCE().root()[STR_LOCAL] && Settings::INSTANCE().root()[STR_LOCAL][STR_REPO])
                pks["MKN_REPO"] = Settings::INSTANCE().root()[STR_LOCAL][STR_REPO].Scalar();
            else
                pks["MKN_REPO"] = kul::user::home(kul::Dir::JOIN(STR_MAIKEN, STR_REPO)).path();

            if(Settings::INSTANCE().root()[STR_LOCAL] && Settings::INSTANCE().root()[STR_LOCAL][STR_MOD_REPO])
                pks["MKN_MOD_REPO"] = Settings::INSTANCE().root()[STR_LOCAL][STR_MOD_REPO].Scalar();
            else
                pks["MKN_MOD_REPO"] = kul::user::home(kul::Dir::JOIN(STR_MAIKEN, STR_MOD_REPO)).path();

            if(Settings::INSTANCE().root()[STR_LOCAL] && Settings::INSTANCE().root()[STR_LOCAL][STR_BIN])
                pks["MKN_BIN"] = Settings::INSTANCE().root()[STR_LOCAL][STR_BIN].Scalar();
            if(Settings::INSTANCE().root()[STR_LOCAL] && Settings::INSTANCE().root()[STR_LOCAL][STR_LIB])
                pks["MKN_LIB"] = Settings::INSTANCE().root()[STR_LOCAL][STR_LIB].Scalar();

            evs["MKN_OBJ"] = "o";
            std::string obj = kul::env::GET("MKN_OBJ");
            if(!obj.empty()) evs["MKN_OBJ"] = obj;
        }
    public:
        const std::string& args() const  { return aa;}
        void args(const std::string& aa) { this->aa = aa;}

        const std::string& runArgs() const  { return ra;}
        void runArgs(const std::string& ra) { this->ra = ra;}

        const kul::hash::map::S2S& jargs() const { return jas;}
        void jargs(const std::string& a, const std::string& b) { jas[a] = b; }

        const std::string& linker() const  { return la;}
        void linker(const std::string& la) { this->la = la;}

        const std::string& allinker() const  { return al;}
        void allinker(const std::string& al) { this->al = al;}

        const bool& dryRun() const { return this->dr;}
        void dryRun(const bool& dr) { this->dr = dr;}

        const bool& update() const { return this->u;}
        void update(const bool& u) { this->u = u;}

        const bool& fupdate() const { return this->f;}
        void fupdate(const bool& f) { this->f = f;}

        const bool& show() const { return this->s;}
        void show(const bool& s){ this->s = s;}

        const bool& shar() const { return this->sh;}
        void shar(const bool& sh){ this->sh = sh;}

        const bool& stat() const  { return this->st;}
        void stat(const bool& st) { this->st = st;}

        const std::string* dependencyString()   const { return dep;}
        void dependencyString(const std::string* dep) { this->dep = dep;}

        const uint16_t& dependencyLevel()  const { return dl;}
        void dependencyLevel(const uint16_t& dl) { this->dl = dl;}

        const uint16_t& threads() const { return ts;}
        void threads(const uint16_t& t) { this->ts = t;}

        const kul::hash::map::S2S& properkeys() const { return pks;}
        void properkeys(const std::string& k, const std::string& v) { pks[k] = v;}

        const kul::hash::map::S2S& envVars() const { return evs;}
        void envVars(const std::string& k, const std::string& v)  { evs[k] = v;}

        void command(const std::string& s)             { cmds.insert(s); }
        const kul::hash::set::String& commands() const { return cmds; }

        void modulePhase(const std::string& s)             { modPhases.insert(s); }
        const kul::hash::set::String& modulePhases() const { return modPhases; }

        static AppVars& INSTANCE(){
            static AppVars instance;
            return instance;
        }
};

class Module;
class ModuleLoader;
class ThreadingCompiler;
class Applications;
class KUL_PUBLISH Application : public Constants{
    friend class Applications;
    friend class ThreadingCompiler;
    protected:
        bool ig = 1, isMod = 0;
        const Application* par = nullptr;
        Application* sup = nullptr;
        kul::code::Mode m;
        std::string arg, main, lang, lnk, out, scr, scv;
        const std::string p;
        kul::Dir bd, inst;
        YAML::Node modCArg, modLArg, modPArg;
        const maiken::Project& proj;
        kul::hash::map::S2T<kul::hash::map::S2S> fs;
        kul::hash::map::S2S includeStamps, itss, ps;
        kul::hash::map::S2T<kul::hash::set::String> args;
        kul::hash::map::S2T<uint16_t> stss;
        std::vector<Application> deps, modDeps;
        std::vector<std::shared_ptr<ModuleLoader>> mods;
        std::vector<kul::cli::EnvVar> evs;
        std::vector<std::string> libs, paths;
        std::vector<std::pair<std::string, bool> > incs, srcs;
        const kul::SCM* scm = 0;

        kul::code::CompilerProcessCapture buildExecutable(const std::vector<std::string>& objects);
        kul::code::CompilerProcessCapture buildLibrary(const std::vector<std::string>& objects);
        void checkErrors(const kul::code::CompilerProcessCapture& cpc) KTHROW(kul::Exception);

        void populateMaps(const YAML::Node& n) KTHROW(kul::Exception);

        void preSetupValidation() KTHROW(Exception);
        void postSetupValidation() KTHROW(Exception);
        void resolveProperties() KTHROW(Exception);
        void resolveLang() KTHROW(Exception);

        void compile(std::vector<std::string>& objects) KTHROW(kul::Exception);
        void build() KTHROW(kul::Exception);
        void pack()  KTHROW(kul::Exception);
        void link()  KTHROW(kul::Exception);
        void link(const std::vector<std::string>& objects) KTHROW(kul::Exception);
        void run(bool dbg);
        void trim();
        void trim(const kul::File& f);

        void scmStatus(const bool& deps = false) KTHROW(kul::scm::Exception);
        void scmUpdate(const bool& f) KTHROW(kul::scm::Exception);
        void scmUpdate(const bool& f, const kul::SCM* scm, const std::string& repo) KTHROW(kul::scm::Exception);

        void setup() KTHROW(kul::Exception);
        void setSuper();
        void showConfig(bool force = 0);
        void cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const KTHROW(kul::Exception);
        void showProfiles();
        void loadTimeStamps() KTHROW(kul::StringException);

        void buildDepVec(const std::string* depVal);
        void buildDepVecRec(std::vector<Application*>& dePs, int16_t i, const kul::hash::set::String& inc);

        void populateMapsFromDependencies() KTHROW(kul::Exception);

        void loadDepOrMod(const YAML::Node& node, const kul::Dir& depOrMod, bool module) KTHROW(kul::Exception);
        kul::Dir resolveDepOrModDirectory(const YAML::Node& d, bool module);
        void popDepOrMod(const YAML::Node& n, std::vector<Application>& vec, const std::string& s, bool module) KTHROW(kul::Exception);

        kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String> > sourceMap();
        kul::hash::set::String inactiveMains();

        bool incSrc(const kul::File& f);
        void addSourceLine (const std::string& o) KTHROW(kul::Exception);
        void addIncludeLine(const std::string& o) KTHROW(kul::Exception);

        void              modCompile(const YAML::Node& modArg){ modCArg = modArg; }
        const YAML::Node& modCompile()                        { return modCArg; }
        void              modLink   (const YAML::Node& modArg){ modLArg = modArg; }
        const YAML::Node& modLink()                           { return modLArg; }
        void              modPack   (const YAML::Node& modArg){ modPArg = modArg; }
        const YAML::Node& modPack()                           { return modPArg; }

        static void showHelp();
    public:
        Application(const maiken::Project& proj, const std::string& profile = "");// : m(kul::code::Mode::NONE), p(profile), proj(proj){}
        ~Application();

        virtual void                                       process()   KTHROW(kul::Exception);
        const kul::Dir&                                    buildDir()            const { return bd; }
        const std::string&                                 profile()             const { return p; }
        const maiken::Project&                             project()             const { return proj;}
        const std::vector<Application>&                    dependencies()        const { return deps; }
        const std::vector<Application>&                    moduleDependencies()  const { return modDeps; }
        const std::vector<std::shared_ptr<ModuleLoader>>&  modules()             const { return mods; }
        const std::vector<kul::cli::EnvVar>&               envVars()             const { return evs; }
        const kul::hash::map::S2T<kul::hash::map::S2S>&    files()               const { return fs; }
        const std::vector<std::string>&                    libraries()           const { return libs;}
        const std::vector<std::pair<std::string, bool> >&  sources()             const { return srcs;}
        const std::vector<std::pair<std::string, bool> >&  includes()            const { return incs;}
        const std::vector<std::string>&                    libraryPaths()        const { return paths;}
        const kul::hash::map::S2S&                         properties()          const { return ps;}
        const kul::hash::map::S2T<kul::hash::set::String>& arguments()           const { return args; }

        std::string                                        baseLibFilename()     const {
            std::string n = project().root()[STR_NAME].Scalar(); 
            return out.empty() ? inst ? p.empty() ? n : n + "_" + p : n : out;
        }

        static Application& CREATE(int16_t argc, char *argv[]) KTHROW(kul::Exception);
};

class Applications{
    friend int ::main(int argc, char* argv[]);
    private:
        kul::hash::map::S2T<kul::hash::map::S2T<Application*>> m_apps;
        std::vector<std::unique_ptr<Application>> m_appPs;
        Applications(){}
        void clear(){
            m_apps.clear();
            m_appPs.clear();
        }
    public:
        static Applications& INSTANCE(){
            static Applications a;
            return a;
        }
        Application* getOrCreate(const maiken::Project& proj, const std::string& _profile = "", bool setup = 1) KTHROW(kul::Exception) {
            std::string pDir(proj.dir().real());
            std::string profile = _profile.empty() ? "@" : _profile;
            if(!m_apps.count(pDir) || !m_apps[pDir].count(profile)){
                auto app = std::make_unique<Application>(proj, _profile);
                auto pp = app.get();
                m_appPs.push_back(std::move(app));
                m_apps[pDir][profile] = pp;
                if(setup){
                    const std::string& cwd(kul::env::CWD());
                    kul::env::CWD(proj.dir());
                    pp->setup();
                    kul::env::CWD(cwd);
                }
            }
            return m_apps[pDir][profile];
        }
};

class ThreadingCompiler : public Constants{
    private:
        bool f;
        kul::Mutex compile;
        kul::Mutex push;
        maiken::Application& app;
        std::queue<std::pair<std::string, std::string> >& sources;
        std::vector<kul::code::CompilerProcessCapture> cpcs;
        std::vector<std::string> incs;
    public:
        ThreadingCompiler(maiken::Application& app, std::queue<std::pair<std::string, std::string> >& sources)
            : f(0), app(app), sources(sources){
                for(const auto& s : app.includes()){
                    kul::Dir d(s.first);
                    const std::string& m(AppVars::INSTANCE().dryRun() ? d.esc() : d.escm());
                    if(!m.empty()) incs.push_back(m);
                    else           incs.push_back(".");
                }
            }
        void operator()() KTHROW(kul::Exception);
        const std::vector<kul::code::CompilerProcessCapture>& processCaptures(){return cpcs;}
};

class SCMGetter{
    private:
        kul::hash::map::S2S valids;
        static bool IS_SOLID(const std::string& r){
            return r.find("://") != std::string::npos || r.find("@") != std::string::npos;
        }
        static SCMGetter& INSTANCE(){
            static SCMGetter s;
            return s;
        }
        static const kul::SCM* GET_SCM(const kul::Dir& d, const std::string& r, bool module);
    public:
        static const std::string REPO(const kul::Dir& d, const std::string& r, bool module){
            if(INSTANCE().valids.count(d.path())) return (*INSTANCE().valids.find(d.path())).second;
            if(IS_SOLID(r)) INSTANCE().valids.insert(d.path(), r);
            else            GET_SCM(d, r, module);
            if(INSTANCE().valids.count(d.path())) return (*INSTANCE().valids.find(d.path())).second;
            KEXCEPT(Exception, "SCM not discovered for project: "+d.path());
        }
        static bool HAS(const kul::Dir& d){
            return (kul::Dir(d.join(".git")) || kul::Dir(d.join(".svn")));
        }
        static const kul::SCM* GET(const kul::Dir& d, const std::string& r, bool module){
            if(IS_SOLID(r)) INSTANCE().valids.insert(d.path(), r);
            if(kul::Dir(d.join(".git"))) return &kul::scm::Manager::INSTANCE().get("git");
            if(kul::Dir(d.join(".svn"))) return &kul::scm::Manager::INSTANCE().get("svn");
            return r.size() ? GET_SCM(d, r, module) : 0;
        }
};

}
#endif /* _MAIKEN_APP_HPP_ */

#include <maiken/module.hpp>
#include <maiken/property.hpp>