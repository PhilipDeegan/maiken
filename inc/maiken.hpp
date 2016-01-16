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
#include "kul/scm.hpp"
#include "kul/proc.hpp"
#include "kul/threads.hpp"
#include "kul/code/compilers.hpp"

#include "maiken/project.hpp"

namespace maiken{

class ThreadingCompiler;

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

class AppVars{
    private:
        bool b = 0, c = 0, d = 0, f = 0, g = 0, l = 0, p = 0, r = 0, s = 0, sh = 0, st = 0, t = 0, u = 0;
        uint16_t dl = 0;
        uint16_t ts = 1;
        std::string aa;
        std::string la;
        kul::hash::map::S2S pks;
        kul::hash::map::S2S jas;
        AppVars(){
            pks["OS"] = KTOSTRING(__KUL_OS__);
        }
    public:
        const std::string& args() const { return aa;}
        void args(const std::string& aa)    { this->aa = aa;}

        const kul::hash::map::S2S& jargs() const { return jas;}
        void jargs(const std::string& a, const std::string& b) { this->jas.insert(a, b);}

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
        void debug(const bool& d)   { this->d = d;}

        const bool& fupdate() const { return this->f;}
        void fupdate(const bool& f)   { this->f = f;}

        const bool& link() const { return this->l;}
        void link(const bool& l) { this->l = l;}

        const bool& run() const { return this->r;}
        void run(const bool& r) { this->r = r;}

        const bool& show() const { return this->s;}
        void show(const bool& s){ this->s = s;}

        const bool& shar() const { return this->sh;}
        void shar(const bool& sh){ this->sh = sh;}

        const bool& trim() const { return this->t;}
        void trim(const bool& t) { this->t = t;}

        const bool& update() const { return this->u;}
        void update(const bool& u)   { this->u = u;}

        const bool& stat() const { return this->st;}
        void stat(const bool& st){ this->st = st;}

        const uint16_t& dependencyLevel()  const { return dl;}
        void dependencyLevel(const uint16_t& dl)  { this->dl = dl;}

        const uint16_t& threads() const { return ts;}
        void threads(const uint16_t& t) { this->ts = t;}

        const kul::hash::map::S2S& properkeys() const { return pks;}

        static AppVars& INSTANCE(){
            static AppVars instance;
            return instance;
        }
};

class ThreadingCompiler;
class Application : public Constants{
    protected:
        bool ig = 1;
        const Application* par = 0;
        kul::code::Mode m;
        std::string arg, main, lang;
        const std::string p;
        kul::Dir bd, inst;
        maiken::Project proj;
        kul::hash::map::S2S2T<std::string> fs;
        std::vector<std::string> libs;
        std::vector<std::pair<std::string, bool> > srcs;
        std::vector<std::pair<std::string, bool> > incs;
        std::vector<std::string> paths;
        kul::hash::map::S2S ps;
        kul::hash::map::S2T<kul::hash::set::String> args;
        kul::hash::map::S2T<uint16_t> stss;
        kul::hash::map::S2S itss;
        kul::hash::map::S2S includeStamps;
        std::vector<kul::cli::EnvVar> evs;
        std::vector<Application> deps;
        std::string scr;
        const kul::SCM* scm = 0;

        Application(const maiken::Project& proj, const std::string profile) : m(kul::code::Mode::NONE), p(profile), proj(proj){}
        Application(const maiken::Project& proj) : m(kul::code::Mode::NONE), proj(proj){}
        void                           buildDepVec();
        void                           buildDepVecRec(std::vector<Application*>& dePs, uint16_t i);
        void                           buildExecutable(const std::vector<std::string>& objects);
        void                           buildLibrary(const std::vector<std::string>& objects);
        void                           checkErrors(const kul::code::CompilerProcessCapture& cpc) throw(kul::Exception);
        const std::vector<std::string> compile() throw(kul::Exception);
        const kul::hash::set::String   inactiveMains();
        void                           populateMaps(const YAML::Node& n);
        void                           populateMapsFromDependencies();
        void                           populateDependencies(const YAML::Node& n) throw(kul::Exception);
        void                           preSetupValidation() throw(Exception);
        void                           postSetupValidation() throw(Exception);
        const kul::Dir                 resolveDependencyDirectory(const YAML::Node& d);
        void                           resolveProperties();
        const std::string              resolveFromProperties(const std::string& s) const;
        void                           build()     throw(kul::Exception);
        void                           link()      throw(kul::Exception);
        void                           run(bool dbg);
        void                           trim();
        void                           trim(const kul::File& f);
        void                           scmStatus(const bool& deps = false) throw(kul::scm::Exception);
        void                           scmUpdate(const bool& f) throw(kul::scm::Exception);
        void                           scmUpdate(const bool& f, const kul::SCM* scm, const std::string& repo) throw(kul::scm::Exception);
        void                           setup();
        void                           showConfig();
        void                           cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const throw(kul::Exception);
        void                           showProfiles();
        void                           loadTimeStamps() throw (kul::TypeException);
        bool                           incSrc(const kul::File& f);
        const kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String> > sourceMap();

        void addSourceLine (const std::string& o) throw (kul::TypeException);
        void addIncludeLine(const std::string& o) throw (kul::TypeException);

        static void                                 showHelp();
    public:
        static Application create(int16_t argc, char *argv[]) throw(kul::Exception);
        virtual void                                       process()   throw(kul::Exception);
        const kul::Dir&                                    buildDir()      const { return bd; }
        const std::string&                                 profile()       const { return p; }
        const maiken::Project&                             project()       const { return proj;}
        const std::vector<Application>&                    dependencies()  const { return deps; }
        const std::vector<kul::cli::EnvVar>&               envVars()       const { return evs; }
        const kul::hash::map::S2S2T<std::string>&          files()         const { return fs; }
        const std::vector<std::string>&                    libraries()     const { return libs;}
        const std::vector<std::pair<std::string, bool> >&  sources()       const { return srcs;}
        const std::vector<std::pair<std::string, bool> >&  includes()      const { return incs;}
        const std::vector<std::string>&                    libraryPaths()  const { return paths;}
        const kul::hash::map::S2S&                         properties()    const { return ps;}
        const kul::hash::map::S2T<kul::hash::set::String>& arguments()     const { return args; }

        friend class ThreadingCompiler;
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
                    const std::string& m(kul::Dir(s.first).escm());
                    if(!m.empty()) incs.push_back(m);
                }
            }
        void operator()() throw(kul::Exception){
            std::pair<std::string, std::string> p;
            {
                kul::ScopeLock lock(compile);
                p = sources.front();
                sources.pop();
            }
            const std::string src(p.first);
            const std::string obj(p.second);
            if(!f){
                const std::string& fileType = src.substr(src.rfind(".") + 1);
                const std::string& compiler = (*(*app.files().find(fileType)).second.find(COMPILER)).second;
                std::vector<std::string> args;
                if(app.arguments().count(fileType) > 0)
                    for(const std::string& o : (*app.arguments().find(fileType)).second)
                        for(const auto& s : kul::cli::asArgs(o))
                            args.push_back(s);
                for(const auto& s : kul::cli::asArgs(app.arg)) args.push_back(s);
                std::string cmd = compiler + " " + AppVars::INSTANCE().args();
                if(AppVars::INSTANCE().jargs().count(fileType) > 0)
                    cmd += " " + (*AppVars::INSTANCE().jargs().find(fileType)).second;
                // WE CHECK BEFORE USING THIS THAT A COMPILER EXISTS FOR EVERY FILE
                if(kul::LogMan::INSTANCE().inf() && !kul::LogMan::INSTANCE().dbg())
                    KOUT(NON) << compiler << " : " << src;
                const kul::code::CompilerProcessCapture& cpc = kul::code::Compilers::INSTANCE().get(compiler)->compileSource(cmd, args, incs, src, obj, app.m);
                kul::ScopeLock lock(push);
                cpcs.push_back(cpc);
                if(cpc.exception()) f = 1;
            }
        }
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
        static const kul::SCM* GET_SCM(const kul::Dir& d, const std::string& r){
            std::vector<std::string> repos;
            if(IS_SOLID(r)) repos.push_back(r);
            else
                for(const std::string& s : Settings::INSTANCE().remoteRepos()) repos.push_back(s + r);
            for(const auto& repo : repos){
                try{
                    kul::Process g("git");
                    kul::ProcessCapture gp(g);
                    std::string r1(repo);
                    if(repo.find("http") != std::string::npos && repo.find("@") == std::string::npos)
                        r1 = repo.substr(0, repo.find("//") + 2) + "u:p@" + repo.substr(repo.find("//") + 2);
                    g.arg("ls-remote").arg(r1).start();
                    if(!gp.errs().size()) {
                        INSTANCE().valids.insert(d.path(), repo);
                        return &kul::scm::Manager::INSTANCE().get("git");
                    }
                }catch(const kul::proc::ExitException& e){}
                try{
                    kul::Process s("svn");
                    kul::ProcessCapture sp(s);
                    s.arg("ls").arg(repo).start();
                    if(!sp.errs().size()) {
                        INSTANCE().valids.insert(d.path(), repo);
                        return &kul::scm::Manager::INSTANCE().get("svn");
                    }
                }catch(const kul::proc::ExitException& e){}
            }
            std::stringstream ss;
            for(const auto& s : repos) ss << s << "\n";
            KEXCEPT(Exception, "SCM not found or not supported type(git/svn) for repo(s)\n"+ss.str()+"project:"+d.path());
        }
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


