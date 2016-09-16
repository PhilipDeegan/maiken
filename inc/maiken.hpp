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

#include "maiken/project.hpp"

namespace maiken{

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class AppVars : public Constants{
    private:
        bool b = 0, c = 0, d = 0, dr = 0, f = 0, g = 0, l = 0, p = 0, pk = 0, r = 0, s = 0, sh = 0, st = 0, t = 0, u = 0;
        uint16_t dl = 0, ts = 1;
        std::string aa, la;
        kul::hash::map::S2S evs, jas, pks;
        AppVars(){
            pks["OS"]   = KTOSTRING(__KUL_OS__);
            pks["HOME"] = kul::user::home().path();
            pks["DATETIME"] = kul::DateTime::NOW();
            pks["TIMESTAMP"] = std::time(NULL);
            if(Settings::INSTANCE().root()[LOCAL] && Settings::INSTANCE().root()[LOCAL][REPO])
                pks["MKN_REPO"] = Settings::INSTANCE().root()[LOCAL][REPO].Scalar();
            else
                pks["MKN_REPO"] = kul::user::home(kul::Dir::JOIN(MAIKEN, REPO)).path();

        }
    public:
        const std::string& args() const  { return aa;}
        void args(const std::string& aa) { this->aa = aa;}

        const kul::hash::map::S2S& jargs() const { return jas;}
        void jargs(const std::string& a, const std::string& b) { jas[a] = b; }

        const std::string& linker() const  { return la;}
        void linker(const std::string& la) { this->la = la;}

        const bool& build() const { return this->b;}
        void build(const bool& b) { this->b = b;}

        const bool& clean() const { return this->c;}
        void clean(const bool& c) { this->c = c;}

        const bool& compile() const { return this->p;}
        void compile(const bool& p) { this->p = p;}

        const bool& dbg() const { return this->g;}
        void dbg(const bool& g)   { this->g = g;}

        const bool& debug() const { return this->d;}
        void debug(const bool& d) { this->d = d;}

        const bool& dryRun() const { return this->dr;}
        void dryRun(const bool& dr) { this->dr = dr;}

        const bool& fupdate() const { return this->f;}
        void fupdate(const bool& f) { this->f = f;}

        const bool& link() const { return this->l;}
        void link(const bool& l) { this->l = l;}

        const bool& run() const { return this->r;}
        void run(const bool& r) { this->r = r;}

        const bool& pack() const { return this->pk;}
        void pack(const bool& pk) { this->pk = pk;}

        const bool& show() const { return this->s;}
        void show(const bool& s){ this->s = s;}

        const bool& shar() const { return this->sh;}
        void shar(const bool& sh){ this->sh = sh;}

        const bool& trim() const { return this->t;}
        void trim(const bool& t) { this->t = t;}

        const bool& update() const { return this->u;}
        void update(const bool& u) { this->u = u;}

        const bool& stat() const  { return this->st;}
        void stat(const bool& st) { this->st = st;}

        const uint16_t& dependencyLevel()  const { return dl;}
        void dependencyLevel(const uint16_t& dl) { this->dl = dl;}

        const uint16_t& threads() const { return ts;}
        void threads(const uint16_t& t) { this->ts = t;}

        const kul::hash::map::S2S& properkeys() const { return pks;}
        void properkeys(const std::string& k, const std::string& v)  { pks[k] = v;}

        const kul::hash::map::S2S& envVars() const { return evs;}
        void envVars(const std::string& k, const std::string& v)  { evs[k] = v;}

        static AppVars& INSTANCE(){
            static AppVars instance;
            return instance;
        }
};

class ThreadingCompiler;
class Application : public Constants{
    friend class ThreadingCompiler;
    protected:
        bool ig = 1;
        const Application* par = 0;
        std::shared_ptr<Application> sup;
        kul::code::Mode m;
        std::string arg, main, lang, lnk, scr, scv;
        const std::string p;
        kul::Dir bd, inst;
        maiken::Project proj;
        kul::hash::map::S2T<kul::hash::map::S2S> fs;
        kul::hash::map::S2S includeStamps, itss, ps;
        kul::hash::map::S2T<kul::hash::set::String> args;
        kul::hash::map::S2T<uint16_t> stss;
        std::vector<Application> deps;
        std::vector<kul::cli::EnvVar> evs;
        std::vector<std::string> libs, paths;
        std::vector<std::pair<std::string, bool> > incs, srcs;
        const kul::SCM* scm = 0;
        
        void buildDepVec(const std::string* depVal);
        void buildDepVecRec(std::vector<Application*>& dePs, int16_t i, const kul::hash::set::String& inc);
        kul::code::CompilerProcessCapture buildExecutable(const std::vector<std::string>& objects);
        kul::code::CompilerProcessCapture buildLibrary(const std::vector<std::string>& objects);
        void checkErrors(const kul::code::CompilerProcessCapture& cpc) throw(kul::Exception);
        void populateMaps(const YAML::Node& n);
        void populateMapsFromDependencies();
        void populateDependencies(const YAML::Node& n) throw(kul::Exception);
        void preSetupValidation() throw(Exception);
        void postSetupValidation() throw(Exception);
        void resolveProperties();
        void build() throw(kul::Exception);
        void pack() throw(kul::Exception);
        void link()  throw(kul::Exception);
        void run(bool dbg);
        void trim();
        void trim(const kul::File& f);
        void scmStatus(const bool& deps = false) throw(kul::scm::Exception);
        void scmUpdate(const bool& f) throw(kul::scm::Exception);
        void scmUpdate(const bool& f, const kul::SCM* scm, const std::string& repo) throw(kul::scm::Exception);
        void setup();
        void setSuper(Application* app);
        void showConfig(bool force = 0);
        void cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const throw(kul::Exception);
        void showProfiles();
        void loadTimeStamps() throw (kul::StringException);
        bool incSrc(const kul::File& f);
        kul::Dir    resolveDependencyDirectory(const YAML::Node& d);
        kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String> > sourceMap();
        std::vector<std::string> compile() throw(kul::Exception);
        kul::hash::set::String   inactiveMains();

        void addSourceLine (const std::string& o) throw (kul::StringException);
        void addIncludeLine(const std::string& o) throw (kul::StringException);

        static void showHelp();
    public:
        Application(const maiken::Project& proj, const std::string profile);// : m(kul::code::Mode::NONE), p(profile), proj(proj){}
        Application(const maiken::Project& proj); // : m(kul::code::Mode::NONE), proj(proj);//{}
        ~Application();

        virtual void                                       process()   throw(kul::Exception);
        const kul::Dir&                                    buildDir()      const { return bd; }
        const std::string&                                 profile()       const { return p; }
        const maiken::Project&                             project()       const { return proj;}
        const std::vector<Application>&                    dependencies()  const { return deps; }
        const std::vector<kul::cli::EnvVar>&               envVars()       const { return evs; }
        const kul::hash::map::S2T<kul::hash::map::S2S>&    files()         const { return fs; }
        const std::vector<std::string>&                    libraries()     const { return libs;}
        const std::vector<std::pair<std::string, bool> >&  sources()       const { return srcs;}
        const std::vector<std::pair<std::string, bool> >&  includes()      const { return incs;}
        const std::vector<std::string>&                    libraryPaths()  const { return paths;}
        const kul::hash::map::S2S&                         properties()    const { return ps;}
        const kul::hash::map::S2T<kul::hash::set::String>& arguments()     const { return args; }
        std::string                                        resolveFromProperties(const std::string& s) const;

        static std::shared_ptr<Application> CREATE(int16_t argc, char *argv[]) throw(kul::Exception);
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
        void operator()() throw(kul::Exception);
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
        static const kul::SCM* GET_SCM(const kul::Dir& d, const std::string& r);
    public:
        static const std::string REPO(const kul::Dir& d, const std::string& r){
            if(INSTANCE().valids.count(d.path())) return (*INSTANCE().valids.find(d.path())).second;
            if(IS_SOLID(r)) INSTANCE().valids.insert(d.path(), r);
            else            GET_SCM(d, r);
            if(INSTANCE().valids.count(d.path())) return (*INSTANCE().valids.find(d.path())).second;
            KEXCEPT(Exception, "SCM not discovered for project: "+d.path());
        }
        static bool HAS(const kul::Dir& d){
            return (kul::Dir(d.join(".git")) || kul::Dir(d.join(".svn")));
        }
        static const kul::SCM* GET(const kul::Dir& d, const std::string& r){
            if(IS_SOLID(r)) INSTANCE().valids.insert(d.path(), r);
            if(kul::Dir(d.join(".git"))) return &kul::scm::Manager::INSTANCE().get("git");
            if(kul::Dir(d.join(".svn"))) return &kul::scm::Manager::INSTANCE().get("svn");
            return r.size() ? GET_SCM(d, r) : 0;
        }
};

}
#endif /* _MAIKEN_APP_HPP_ */


