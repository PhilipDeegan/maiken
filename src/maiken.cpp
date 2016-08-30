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
#include <queue>

#include "maiken.hpp"

std::shared_ptr<maiken::Application> maiken::Application::CREATE(int16_t argc, char *argv[]) throw(kul::Exception){
    using namespace kul::cli;

    std::vector<Arg> argV { Arg('a', ARG    ,  ArgType::STRING),
                            Arg('j', JARG   ,  ArgType::STRING),
                            Arg('l', LINKER ,  ArgType::STRING),
                            Arg('d', MKN_DEP,  ArgType::MAYBE),
                            Arg('E', ENV    ,  ArgType::STRING),
                            Arg('p', PROFILE,  ArgType::STRING), 
                            Arg('P', PROPERTY, ArgType::STRING), 
                            Arg('t', THREADS,  ArgType::MAYBE),
                            Arg('u', SCM_UPDATE), Arg('U', SCM_FUPDATE),
                            Arg('v', VERSION),    Arg('s', SCM_STATUS),
                            Arg('K', STATIC),      
                            Arg('R', DRY_RUN),    Arg('S', SHARED),
                            Arg('D', DEBUG),      Arg('C', DIRECTORY, ArgType::STRING),
                            Arg('x', SETTINGS, ArgType::STRING),   Arg('h', HELP)};
    std::vector<Cmd> cmdV { Cmd(INIT),     Cmd(MKN_INC), Cmd(MKN_SRC), Cmd(MKN_DEPS),
                            Cmd(CLEAN),    Cmd(BUILD),   Cmd(COMPILE),
                            Cmd(LINK),     Cmd(RUN),     Cmd(DBG), Cmd(PACK),
                            Cmd(PROFILES), Cmd(TRIM),    Cmd(INFO)};
    Args args(cmdV, argV);
    try{
        args.process(argc, argv);
    }catch(const kul::cli::ArgNotFoundException& e){
        showHelp();
        KEXIT(0, "");
    }
    if(args.empty() || (args.size() == 1 && args.has(DIRECTORY))){
        if(args.size() == 1 && args.has(DIRECTORY)){
            kul::Dir d(args.get(DIRECTORY));
            if(!d) KEXCEPTION("DIRECTORY DOES NOT EXIST: " + args.get(DIRECTORY));
            kul::env::CWD(d);
        }
        kul::File yml("mkn.yaml");
        if(yml){
            kul::io::Reader reader(yml);
            const char* c = reader.readLine();
            const std::string s(c ? c : ""); 
            if(s.size() > 3 && s.substr(0, 3) == "#! "){
                std::string line(s.substr(3));
                if(!line.empty()){
                    std::vector<std::string> lineArgs(kul::cli::asArgs(line));
                    std::vector<char*> lineV;
                    lineV.push_back(argv[0]);
                    for(size_t i = 0; i < lineArgs.size(); i++) lineV.push_back(&lineArgs[i][0]);
                    return CREATE(lineV.size(), &lineV[0]);
                }
            }
        }
        showHelp();
        KEXIT(0, "");
    }
    if(args.empty() || args.has(HELP)){
        showHelp();
        KEXIT(0, "");
    }
    if(args.has(VERSION)){
        KOUT(NON) << KTOSTRING(_MKN_VERSION_) << " (" << KTOSTRING(__KUL_OS__) << ")";
        KEXIT(0, "");
    }
    if(args.has(INIT)){
        NewProject p;
        KEXIT(0, "");
    }
    if(args.has(DIRECTORY)) {
        kul::Dir d(args.get(DIRECTORY));
        if(!d) KEXCEPTION("DIRECTORY DOES NOT EXIST: " + args.get(DIRECTORY));
        kul::env::CWD(args.get(DIRECTORY));
    }
    Project project(Project::CREATE());
    std::string profile;
    if(args.has(PROFILE)){
        profile = args.get(PROFILE);
        bool f = 0;
        for(const auto& n : project.root()[PROFILE]){
            f = n[NAME].Scalar() == profile;
            if(f) break;
        }
        if(!f) KEXCEPT(Exception, "profile does not exist");
    }
    if(args.has(SETTINGS) && !Settings::SET(args.get(SETTINGS)))
        KEXCEPT(Exception, "Unable to set specific settings xml");
    else Settings::INSTANCE();
    auto app = std::make_shared<Application>(project, profile);
    auto& a = *app.get();
    if(args.has(PROFILES)){
        a.showProfiles();
        KEXIT(0, "");
    }
    a.ig = 0;
    if(args.has(DRY_RUN))       AppVars::INSTANCE().dryRun(true);
    if(args.has(SHARED))        AppVars::INSTANCE().shar(true);
    if(args.has(STATIC))        AppVars::INSTANCE().stat(true);
    if(AppVars::INSTANCE().shar() && AppVars::INSTANCE().stat())
        KEXCEPT(Exception, "Cannot specify shared and static simultaneously");
    if(args.has(DBG))           AppVars::INSTANCE().dbg(true);
    if(args.has(DEBUG))         AppVars::INSTANCE().debug(true);
    if(args.has(SCM_FUPDATE))   AppVars::INSTANCE().fupdate(true);
    if(args.has(SCM_UPDATE))    AppVars::INSTANCE().update(true);
    if(project.root()[SCM])     a.scr = project.root()[SCM].Scalar();

    if(args.has(MKN_DEP)) AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());

    auto splitArgs = [](const std::string& s, const std::string& t, const std::function<void(const std::string&, const std::string&)>& f){
        for(const auto& p : kul::String::ESC_SPLIT(s, ',')){
            std::vector<std::string> ps;
            kul::String::ESC_SPLIT(p, '=', ps);
            if(ps.size() > 2) KEXCEPTION(t + " override invalid, escape extra \"=\"");
            f(ps[0], ps[1]);
        }
    };

    if(args.has(PROPERTY)) 
        splitArgs(args.get(PROPERTY), 
            "property",
            std::bind(
                (void(AppVars::*)(const std::string&, const std::string&)) &AppVars::properkeys, 
                std::ref(AppVars::INSTANCE()), 
                std::placeholders::_1, 
                std::placeholders::_2));
    if(args.has(ENV))      
        splitArgs(args.get(ENV),      
            "environment", 
            std::bind(
                (void(AppVars::*)(const std::string&, const std::string&)) &AppVars::envVars, 
                std::ref(AppVars::INSTANCE()), 
                std::placeholders::_1, 
                std::placeholders::_2));

    a.setSuper(0);
    a.setup();
    a.buildDepVec(args.has(MKN_DEP) ? &args.get(MKN_DEP) : 0);

    if(args.has(ARG)) AppVars::INSTANCE().args(args.get(ARG));
    if(args.has(LINKER)) AppVars::INSTANCE().linker(args.get(LINKER));
    if(args.has(THREADS)){
        if(args.get(THREADS).size())
            AppVars::INSTANCE().threads(kul::String::UINT16(args.get(THREADS)));
        else AppVars::INSTANCE().threads(kul::cpu::threads());
    }
    if(args.has(JARG)){
        try{
            YAML::Node node = YAML::Load(args.get(JARG));
            for(YAML::const_iterator it = node.begin(); it != node.end(); ++it)
                for(const auto& s : kul::String::SPLIT(it->first.Scalar(), ':'))
                    AppVars::INSTANCE().jargs(s, it->second.Scalar());
        }catch(const std::exception& e){ KEXCEPTION("JSON args failed to parse"); }
    }

    if(args.has(MKN_DEPS)){
        std::vector<Application*> v;
        for(auto app = a.deps.rbegin(); app != a.deps.rend(); ++app){
            const std::string& s((*app).project().dir().real());
            auto it = std::find_if(v.begin(), v.end(), [&s](const Application* app) { return (*app).project().dir().real() == s;});
            if (it == v.end()) v.push_back(&(*app));
        }
        for(auto* app : v) KOUT(NON) <<  (*app).project().dir();
        KEXIT(0, "");
    }
    if(args.has(MKN_INC)){
        for(const auto& p : a.includes())
            KOUT(NON) << p.first;
        KEXIT(0, "");
    }
    if(args.has(MKN_SRC)){
        for(const auto& p1 : a.sourceMap())
            for(const auto& p2 : p1.second)
                for(const auto& p3 : p2.second)
                    KOUT(NON) << kul::File(p3).full();
        for(auto app = a.deps.rbegin(); app != a.deps.rend(); ++app)
            for(const auto& p1 : (*app).sourceMap())
                if(!(*app).ig)
                    for(const auto& p2 : p1.second)
                        for(const auto& p3 : p2.second)
                            KOUT(NON) << kul::File(p3).full();
        KEXIT(0, "");
    }
    if(args.has(SCM_STATUS)){
        a.scmStatus(args.has(MKN_DEP));
        KEXIT(0, "");
    }
    if(args.has(INFO)) a.showConfig(1);

    if(args.has(BUILD))     AppVars::INSTANCE().build(true);
    if(args.has(CLEAN))     AppVars::INSTANCE().clean(true);
    if(args.has(COMPILE))   AppVars::INSTANCE().compile(true);
    if(args.has(LINK))      AppVars::INSTANCE().link(true);
    if(args.has(RUN))       AppVars::INSTANCE().run(true);
    if(args.has(TRIM))      AppVars::INSTANCE().trim(true);
    if(args.has(PACK))      AppVars::INSTANCE().pack(true);
    return app;
}

void maiken::Application::process() throw(kul::Exception){
    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app){
        kul::env::CWD((*app).project().dir());
        kul::Dir mkn((*app).buildDir().join(".mkn"));
        if((*app).ig || (*app).srcs.empty()) continue;
        std::vector<std::pair<std::string, std::string> > oldEvs;
        for(const kul::cli::EnvVar& ev : (*app).envVars()){
            const std::string v = kul::env::GET(ev.name());
            oldEvs.push_back(std::pair<std::string, std::string>(ev.name(), v));
            kul::env::SET(ev.name(), ev.toString().c_str());
        }
        if(AppVars::INSTANCE().clean() && (*app).buildDir().is()){
            (*app).buildDir().rm();
            mkn.rm();
        }

        (*app).loadTimeStamps();
        if(AppVars::INSTANCE().trim())  (*app).trim();
        if(AppVars::INSTANCE().build()) (*app).build();
        else{
            if(AppVars::INSTANCE().compile())(*app).compile();
            if(AppVars::INSTANCE().link())   (*app).link();
        }
        for(const std::pair<std::string, std::string>& oldEv : oldEvs)
            kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
    }
    kul::env::CWD(this->project().dir());
    if(!this->ig){
        for(const kul::cli::EnvVar& ev : this->envVars()) kul::env::SET(ev.name(), ev.toString().c_str());
        if(AppVars::INSTANCE().clean() && this->buildDir().is()){
            this->buildDir().rm();
            kul::Dir(this->buildDir().join(".mkn")).rm();
        }
        loadTimeStamps();
        if(AppVars::INSTANCE().trim())  this->trim();
        if(AppVars::INSTANCE().build()) this->build();
        else{
            if(AppVars::INSTANCE().compile())   this->compile();
            if(AppVars::INSTANCE().link())      this->link();
        }
    }

    if(AppVars::INSTANCE().pack()) pack();
    if(AppVars::INSTANCE().run() || AppVars::INSTANCE().dbg()) run(AppVars::INSTANCE().dbg());
}

void maiken::Application::setup(){
    if(AppVars::INSTANCE().update() || AppVars::INSTANCE().fupdate()) {
        scmUpdate(AppVars::INSTANCE().fupdate());
        proj.reload();
    }
    if(scr.empty()) scr = project().root()[NAME].Scalar();

    this->resolveProperties();
    this->preSetupValidation();
    std::string buildD = kul::Dir::JOIN(BIN, p);
    if(p.empty()) buildD = AppVars::INSTANCE().debug() ? kul::Dir::JOIN(BIN, DEBUG) : kul::Dir::JOIN(BIN, BUILD);
    this->bd = kul::Dir(project().dir().join(buildD));
    std::string profile(p);
    std::vector<YAML::Node> nodes;
    if(profile.empty()){
        nodes.push_back(project().root());
        profile = project().root()[NAME].Scalar();
    }
    if(project().root()[PROFILE])
        for (std::size_t i=0;i < project().root()[PROFILE].size(); i++)
            nodes.push_back(project().root()[PROFILE][i]);

    using namespace kul::cli;
    for(const YAML::Node& c : Settings::INSTANCE().root()[ENV]){
        EnvVarMode mode = EnvVarMode::APPE;
        if      (c[MODE].Scalar().compare(APPEND)   == 0) mode = EnvVarMode::APPE;
        else if (c[MODE].Scalar().compare(PREPEND)  == 0) mode = EnvVarMode::PREP;
        else if (c[MODE].Scalar().compare(REPLACE)  == 0) mode = EnvVarMode::REPL;
        else KEXCEPT(Exception, "Unhandled EnvVar mode: " + c[MODE].Scalar());
        evs.push_back(EnvVar(c[NAME].Scalar(), c[VALUE].Scalar(), mode));
    }

    bool c = 1;
    while(c){
        c = 0;
        for (const auto& n : nodes) {
            if(n[NAME].Scalar() != profile) continue;
            for(const auto& dep : n[MKN_DEP]) {
                const std::string& cwd(kul::env::CWD());
                kul::Dir projectDir(resolveDependencyDirectory(dep));
                if(!projectDir.is()){
                    KOUT(NON) << MKN_PROJECT_NOT_FOUND << projectDir;
                    kul::env::CWD(this->project().dir());
                    const std::string& tscr(dep[SCM] ? resolveFromProperties(dep[SCM].Scalar()) : dep[NAME].Scalar());
                    const std::string& v(dep[VERSION] ? resolveFromProperties(dep[VERSION].Scalar()) : "");
                    KOUT(NON) << SCMGetter::GET(projectDir, tscr)->co(projectDir.path(), SCMGetter::REPO(projectDir, tscr), v);
                    kul::env::CWD(projectDir);
                    if(_MKN_REMOTE_EXEC_){
#ifdef _WIN32
                        if(kul::File("mkn.bat").is() 
                                && kul::proc::Call("mkn.bat", AppVars::INSTANCE().envVars()).run()) 
                            KEXCEPTION("ERROR in "+projectDir.path()+"/mkn.bat");
#else
                        if(kul::File("mkn."+std::string(KTOSTRING(__KUL_OS__))+".sh").is() 
                            && kul::proc::Call("sh mkn."+std::string(KTOSTRING(__KUL_OS__))+".sh", AppVars::INSTANCE().envVars()).run())
                            KEXCEPTION("ERROR in "+projectDir.path()+"mkn."+std::string(KTOSTRING(__KUL_OS__))+".sh");
                        else
                        if(kul::File("mkn.sh").is() && kul::proc::Call("sh mkn.sh", AppVars::INSTANCE().envVars()).run()) 
                            KEXCEPTION("ERROR in "+projectDir.path()+"/mkn.sh");
#endif
                    }
                    kul::env::CWD(this->project().dir());
                }
                kul::env::CWD(cwd);
            }
            populateMaps(n);
            populateDependencies(n);
            profile = n[PARENT] ? resolveFromProperties(n[PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }

    if(Settings::INSTANCE().root()[MKN_INC])
        for(const auto& s : kul::String::SPLIT(Settings::INSTANCE().root()[MKN_INC].Scalar(), ' '))
            if(s.size()){
                kul::Dir d(resolveFromProperties(s));
                if(d) incs.push_back(std::make_pair(d.real(), false));
                else  KEXCEPTION("include does not exist\n"+d.path()+"\n"+Settings::INSTANCE().file());
            }
    if(Settings::INSTANCE().root()[PATH])
        for(const auto& s : kul::String::SPLIT(Settings::INSTANCE().root()[PATH].Scalar(), ' '))
            if(s.size()){
                kul::Dir d(resolveFromProperties(s));
                if(d) paths.push_back(d.escr());
                else  KEXCEPTION("library path does not exist\n"+d.path()+"\n"+Settings::INSTANCE().file());
            }

    this->populateMapsFromDependencies();
    std::vector<std::string> fileStrings{ARCHIVER, COMPILER, LINKER};
    for(const auto& c : Settings::INSTANCE().root()[FILE])
        for(const std::string& s : fileStrings)
            for(const auto& t : kul::String::SPLIT(c[TYPE].Scalar(), ':'))
                if(fs[t].count(s) == 0 && c[s])
                    fs[t].insert(s, c[s].Scalar());

    this->postSetupValidation();
    profile = p.size() ? p : project().root()[NAME].Scalar();
    bool nm = 1;
    c = 1;
    while(c){
        c = 0;
        for(const auto& n : nodes){
            if(n[NAME].Scalar() != profile) continue;
            if(n[MODE] && nm){
                m = n[MODE].Scalar() == STATIC ? kul::code::Mode::STAT
                : n[MODE].Scalar() == SHARED ? kul::code::Mode::SHAR
                : kul::code::Mode::NONE;
                nm = 0;
            }
            if(main.empty() && n[MAIN]) main = n[MAIN].Scalar();
            if(lang.empty() && n[LANG]) lang = n[LANG].Scalar();
            profile = n[PARENT] ? resolveFromProperties(n[PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }
    if(main.empty() && lang.empty()){
        const auto& mains(inactiveMains());
        if(mains.size()) lang = (*mains.begin()).substr((*mains.begin()).rfind(".")+1);
        else
        if(sources().size()) KEXCEPTION("no main or lang tag found and cannot deduce language\n" + project().dir().path());
    }
    if(par){
        if(!main.empty() && lang.empty()) lang = main.substr(main.rfind(".")+1);
        main.clear();
    }
    if(nm){
        if(AppVars::INSTANCE().shar()) m = kul::code::Mode::SHAR;
        else
        if(AppVars::INSTANCE().stat()) m = kul::code::Mode::STAT;
    }
    profile = p.size() ? p : project().root()[NAME].Scalar();
    c = 1;
    while(c){
        c = 0;
        for(const auto& n : nodes){
            if(n[NAME].Scalar() != profile) continue;
            if(inst.path().empty()){
                if(Settings::INSTANCE().root()[LOCAL]
                    && Settings::INSTANCE().root()[LOCAL][BIN]
                    && !main.empty())
                    inst = kul::Dir(Settings::INSTANCE().root()[LOCAL][BIN].Scalar());
                else
                if(Settings::INSTANCE().root()[LOCAL]
                    && Settings::INSTANCE().root()[LOCAL][LIB]
                    && main.empty())
                    inst = kul::Dir(Settings::INSTANCE().root()[LOCAL][LIB].Scalar());
                else
                if(n[INSTALL]) inst = kul::Dir(resolveFromProperties(n[INSTALL].Scalar()));
                if(!inst.path().empty()){
                    if(!inst && !inst.mk())
                        KEXCEPTION("install tag is not a valid directory\n" + project().dir().path());
                    inst = kul::Dir(inst.real());
                }
            }
            if(n[IF_ARG])
                for(YAML::const_iterator it = n[IF_ARG].begin(); it != n[IF_ARG].end(); ++it){
                    std::string left(it->first.Scalar());
                    if(left.find("_") != std::string::npos){
                        if(left.substr(0, left.find("_")) == KTOSTRING(__KUL_OS__))
                            left = left.substr(left.find("_") + 1);
                        else continue;
                    }
                    std::vector<std::string> ifArgs;
                    for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                        ifArgs.push_back(resolveFromProperties(s));
                    if(lang.empty() && left == BIN) for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(main.empty() && left == LIB) for(const auto& s : ifArgs) arg += s + " ";
                    if(m == kul::code::Mode::SHAR && left == SHARED)
                        for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(m == kul::code::Mode::STAT && left == STATIC)
                        for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(left == KTOSTRING(__KUL_OS__)) for(const auto& s : ifArgs) arg += s + " ";
                }
            try{
                if(n[IF_INC])
                    for(YAML::const_iterator it = n[IF_INC].begin(); it != n[IF_INC].end(); ++it)
                        if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                            for(const auto& s : kul::String::LINES(it->second.Scalar()))
                                addIncludeLine(s);
            }catch(const kul::StringException& e){
                KLOG(ERR) << e.what();
                KEXCEPTION("if_inc contains invalid bool value\n"+project().dir().path());
            }
            try{
                if(n[IF_SRC])
                    for(YAML::const_iterator it = n[IF_SRC].begin(); it != n[IF_SRC].end(); ++it)
                        if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                            for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                                addSourceLine(s);
            }catch(const kul::StringException){
                KEXCEPTION("if_src contains invalid bool value\n"+project().dir().path());
            }
            if(n[IF_LIB])
                for(YAML::const_iterator it = n[IF_LIB].begin(); it != n[IF_LIB].end(); ++it)
                    if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                        for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                            if(s.size()) libs.push_back(resolveFromProperties(s));

            profile = n[PARENT] ? resolveFromProperties(n[PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }
}

void maiken::Application::buildDepVec(const std::string* depVal){
    kul::hash::set::String all, ignore, include;
    ignore.insert("+");
    if(depVal){
        if(depVal->size()){
            try{
                AppVars::INSTANCE().dependencyLevel(kul::String::UINT16(*depVal));
            }catch(const kul::StringException& e){
                AppVars::INSTANCE().dependencyLevel(0);
                for(auto s : kul::String::SPLIT(*depVal, ',')){
                    kul::String::TRIM(s);
                    kul::String::REPLACE_ALL(s, " ", "");
                    include.insert(s);
                }
            }
        }else 
            AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());
    }

    if(include.size() == 1 && include.count("+")){
        AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());
        this->ig = 1;
    }

    std::vector<Application*> dePs;
    for(Application& a : deps){
        a.buildDepVecRec(dePs, AppVars::INSTANCE().dependencyLevel(), include);
        const std::string& name(a.project().root()[NAME].Scalar());
        std::stringstream ss;
        ss << name << "[" << (a.p.empty() ? name : a.p) << "]";
        if(AppVars::INSTANCE().dependencyLevel()
            || include.count(name) || include.count(ss.str())) a.ig = 0;
        all.insert(name);
        all.insert(ss.str());
    }
    std::vector<Application> t;
    for(const Application* a : dePs) t.push_back(*a);
    for(const Application& a : t){
        bool f = 0;
        for(const auto& a1: deps)
            if(a.project().dir() == a1.project().dir() && a.p == a1.p){
                f = 1; break;
            }
        if(!f) deps.push_back(a);
    }
    for(const auto& d : include)
        if(!all.count(d) && !ignore.count(d)) KEXCEPTION("Dependency project specified does not exist: "+ d);
    if(include.size() && include.count("+")) this->ig = 1;
}

void maiken::Application::buildDepVecRec(std::vector<Application*>& dePs, int16_t i, const kul::hash::set::String& inc){
    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app){
        maiken::Application& a = (*app);
        const std::string& name(a.project().root()[NAME].Scalar());
        std::stringstream ss;
        ss << name << "[" << (a.p.empty() ? name : a.p) << "]";
        if(i > 0 || inc.count(name) || inc.count(ss.str())) a.ig = 0;
        for(auto app1 = dePs.rbegin(); app1 != dePs.rend(); ++app1){
            maiken::Application* a1 = (*app1);
            if(a.project().dir() == a1->project().dir() && a.p == a1->p){
                dePs.erase(std::remove(dePs.begin(), dePs.end(), &a), dePs.end());
                dePs.push_back(&a);
                break;
            }
        }
        dePs.push_back(&a);
        a.buildDepVecRec(dePs, --i, inc);
    }
}

kul::hash::set::String maiken::Application::inactiveMains(){
    kul::hash::set::String iMs;
    std::string p;
    try{
        p = kul::Dir::REAL(main);
    }catch(const kul::Exception& e){ }
    std::string f;
    try{
        if(project().root()[MAIN]){
            f = kul::Dir::REAL(project().root()[MAIN].Scalar());
            if(p.compare(f) != 0) iMs.insert(f);
        }
    }catch(const kul::Exception& e){ }
    for(const YAML::Node& c : project().root()[PROFILE]){
        try{
            if(c[MAIN]){
                f = kul::Dir::REAL(c[MAIN].Scalar());
                if(p.compare(f) != 0) iMs.insert(f);
            }
        }catch(const kul::Exception& e){ }
    }
    return iMs;
}

kul::Dir maiken::Application::resolveDependencyDirectory(const YAML::Node& n){
    std::string d;
    if(n[LOCAL]) d = resolveFromProperties(n[LOCAL].Scalar());
    else{
        d = (*AppVars::INSTANCE().properkeys().find("MKN_REPO")).second;
        try{
            std::string version(n[VERSION] ? resolveFromProperties(n[VERSION].Scalar()) : "default");
            if(_MKN_REP_VERS_DOT_) kul::String::REPLACE_ALL(version, ".", kul::Dir::SEP());
            std::string name(resolveFromProperties(n[NAME].Scalar()));
            if(_MKN_REP_NAME_DOT_) kul::String::REPLACE_ALL(name, ".", kul::Dir::SEP());
            d = kul::Dir::JOIN(d, kul::Dir::JOIN(name, version));
        }catch(const kul::Exception& e){ KLOG(DBG) << e.debug(); }
    }
    return kul::Dir(d);
}

void maiken::Application::run(bool dbg){
#ifdef _WIN32
    kul::File f(project().root()[NAME].Scalar() + ".exe", inst ? inst : buildDir());
#else
    kul::File f(project().root()[NAME].Scalar(), inst ? inst : buildDir());
#endif
    if(!f) KEXCEPTION("binary does not exist \n" + f.full());

    std::unique_ptr<kul::Process> p;
    if(dbg){
        std::string dbg = kul::env::GET("MKN_DBG");
        if(dbg.empty())
            if(Settings::INSTANCE().root()[LOCAL] && Settings::INSTANCE().root()[LOCAL][DEBUGGER])
                dbg = Settings::INSTANCE().root()[LOCAL][DEBUGGER].Scalar();
        if(dbg.empty()){
#ifdef _WIN32
            p = std::make_unique<kul::Process>("cdb");
            p->arg("-o");
#else
            p = std::make_unique<kul::Process>("gdb");
#endif
        }
        else{
            std::vector<std::string> bits(kul::cli::asArgs(dbg));
            p = std::make_unique<kul::Process>(bits[0]);
            for(uint16_t i = 1; i < bits.size(); i++) p->arg(bits[i]);
        }
        p->arg(f.mini());
    }
    else
        p = std::make_unique<kul::Process>(f.escm());
    for(const auto& s : kul::cli::asArgs(AppVars::INSTANCE().args())) p->arg(s);
    if(m != kul::code::Mode::STAT){
        std::string arg;
        for(const auto& s : libraryPaths()) arg += s + kul::env::SEP();
        arg.pop_back();
#ifdef _WIN32
        kul::cli::EnvVar pa("PATH", arg, kul::cli::EnvVarMode::PREP);
#else
        kul::cli::EnvVar pa("LD_LIBRARY_PATH", arg, kul::cli::EnvVarMode::PREP);
#endif
        KOUT(INF) << pa.name() << " : " << pa.toString();
        p->var(pa.name(), pa.toString());
    }
    for(const auto& ev : AppVars::INSTANCE().envVars())
        p->var(ev.first, kul::cli::EnvVar(ev.first, ev.second, kul::cli::EnvVarMode::PREP).toString());
    KOUT(INF) << (*p);
    if(!AppVars::INSTANCE().dryRun()) p->start();
    KEXIT(0, "");
}

void maiken::Application::trim(){
    for(const auto& s : includes()){
        kul::Dir d(s.first);
        if(d.real().find(project().dir().real()) != std::string::npos)
            for(const kul::File& f : d.files())
                trim(f);
    }
    for(const auto& p1 : sourceMap())
        for(const auto& p2 : p1.second)
            for(const auto& p3 : p2.second){
                const kul::File& f(p3);
                if(f.dir().real().find(project().dir().real()) != std::string::npos) trim(f);
            }
}

void maiken::Application::trim(const kul::File& f){
    kul::File tmp(f.real()+".tmp");
    {
        kul::io::Writer w(tmp);
        kul::io::Reader r(f);
        const char* l = r.readLine();
        if(l){
            std::string s(l);
            while(s.size() && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')) s.pop_back();
            w << s.c_str();
            while((l = r.readLine())){
                w << kul::os::EOL();
                s = *l;
                while(s.size() && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')) s.pop_back();
                w << s.c_str();
            }
        }
    }
    f.rm();
    tmp.mv(f);
}

void maiken::Application::populateMapsFromDependencies(){
    for(auto dep = dependencies().rbegin(); dep != dependencies().rend(); ++dep){
        if((*dep).sources().empty()) continue;
        const std::string& n((*dep).project().root()[NAME].Scalar());
        const std::string& lib = (*dep).inst ? (*dep).p.empty() ? n : (n+"_"+(*dep).p) : n;
        const auto& it(std::find(libraries().begin(), libraries().end(), lib));
        if(it != libraries().end()) libs.erase(it);
        libs.push_back(lib);
    }
    for(auto dep = dependencies().rbegin(); dep != dependencies().rend(); ++dep){
        for(const auto& s : (*dep).includes())
            if(s.second && std::find(includes().begin(), includes().end(), s) == includes().end())
                incs.push_back(std::make_pair(s.first, true));
        for(const std::string& s : (*dep).libraryPaths())
            if(std::find(libraryPaths().begin(), libraryPaths().end(), s) == libraryPaths().end())
                paths.push_back(s);
        for(const std::string& s : (*dep).libraries())
            if(std::find(libraries().begin(), libraries().end(), s) == libraries().end())
                libs.push_back(s);
    }
}

void maiken::Application::populateMaps(const YAML::Node& n){ //IS EITHER ROOT OR PROFILE NODE!
    using namespace kul::cli;
    for(const auto& c : n[ENV]){
        EnvVarMode mode = EnvVarMode::APPE;
        if      (c[MODE].Scalar().compare(APPEND)   == 0) mode = EnvVarMode::APPE;
        else if (c[MODE].Scalar().compare(PREPEND)  == 0) mode = EnvVarMode::PREP;
        else if (c[MODE].Scalar().compare(REPLACE)  == 0) mode = EnvVarMode::REPL;
        else KEXCEPTION("Unhandled EnvVar mode: " + c[MODE].Scalar());
        evs.erase(std::remove_if(evs.begin(), evs.end(),
            [&c](const EnvVar& ev) {return ev.name() == c[NAME].Scalar();}), evs.end());
        evs.push_back(EnvVar(c[NAME].Scalar(), c[VALUE].Scalar(), mode));
    }
    for(const auto& p : AppVars::INSTANCE().envVars()){
        evs.erase(std::remove_if(evs.begin(), evs.end(),
            [&p](const EnvVar& ev) {return ev.name() == p.first;}), evs.end());
        evs.push_back(EnvVar(p.first, p.second, EnvVarMode::PREP));
    }

    if(n[ARG])  for(const auto& o : kul::String::LINES(n[ARG].Scalar()))  arg += resolveFromProperties(o) + " ";
    if(n[LINK]) for(const auto& o : kul::String::LINES(n[LINK].Scalar())) lnk += resolveFromProperties(o) + " ";
    try{
        if(n[MKN_INC]) for(const auto& o : kul::String::LINES(n[MKN_INC].Scalar())) addIncludeLine(o);
    }catch(const kul::StringException){
        KEXCEPT(Exception, "include contains invalid bool value\n"+project().dir().path());
    }
    try{
        if(n[MKN_SRC]) for(const auto& o : kul::String::LINES(n[MKN_SRC].Scalar())) addSourceLine(o);
    }catch(const kul::StringException){
        KEXCEPT(Exception, "source contains invalid bool value\n"+project().dir().path());
    }
    if(n[PATH])
        for(const auto& s : kul::String::SPLIT(n[PATH].Scalar(), ' '))
            if(s.size()){
                kul::Dir d(resolveFromProperties(s));
                if(d) paths.push_back(d.escr());
                else KEXCEPTION("library path does not exist\n"+d.path()+"\n"+project().dir().path());
            }

    if(n[MKN_LIB])
        for(const auto& s : kul::String::SPLIT(n[MKN_LIB].Scalar(), ' '))
            if(s.size()) libs.push_back(resolveFromProperties(s));

    for(const std::string& s : libraryPaths())
        if(!kul::Dir(s).is()) KEXCEPTION(s + " is not a valid directory\n"+project().dir().path());
}

void maiken::Application::populateDependencies(const YAML::Node& n) throw(kul::Exception){
    std::vector<std::pair<std::string, std::string>> apps;
    for(const auto& dep : n[MKN_DEP]){
        const kul::Dir& projectDir = resolveDependencyDirectory(dep);
        bool f = false;
        for(const Application& a : this->dependencies())
            if(projectDir == a.project().dir() && p == a.p){
                f = true; break;
            }
        if(f) continue;
        const maiken::Project c(maiken::Project::CREATE(projectDir));

        if(dep[PROFILE]){
            for(auto s : kul::String::SPLIT(resolveFromProperties(dep[PROFILE].Scalar()), ' ')){
                if(s.empty()) continue;
                f = 0;
                if(s == "@") s = "";
                else
                    for(const auto& node : c.root()[PROFILE])
                        if(node[NAME].Scalar() == s){
                            f = 1;
                            break;
                        }
                    
                if(!f && !s.empty()) 
                    KEXCEPTION("profile does not exist\n"+s+"\n"+project().dir().path());
                Application app(c, s);
                app.par = this;
                if(dep[SCM]) app.scr = resolveFromProperties(dep[SCM].Scalar());
                if(dep[VERSION]) app.scv = resolveFromProperties(dep[VERSION].Scalar());
                this->deps.push_back(app);
                apps.push_back(std::make_pair(app.project().dir().path(), app.p));
            }
        }else{
            Application app(c, "");
            app.par = this;
            if(dep[SCM]) app.scr = resolveFromProperties(dep[SCM].Scalar());
            if(dep[VERSION]) app.scv = resolveFromProperties(dep[VERSION].Scalar());
            this->deps.push_back(app);
            apps.push_back(std::make_pair(app.project().dir().path(), app.p));
        }
    }
    if(n[SELF])
        for(const auto& s : kul::String::SPLIT(resolveFromProperties(n[SELF].Scalar()), ' ')){
            Application app(project(), s);
            app.par = this;
            app.scr = scr;
            this->deps.push_back(app);
            apps.push_back(std::make_pair(app.project().dir().path(), app.p));
        }
    cyclicCheck(apps);
    for(auto& app : deps){
        if(app.buildDir().path().size()) continue;
        kul::env::CWD(app.project().dir());
        app.setSuper(this);
        app.setup();
        if(app.project().root()[SCM]) app.scr = app.resolveFromProperties(app.project().root()[SCM].Scalar());
        if(!app.sources().empty()){
            app.buildDir().mk();
            app.paths.push_back(app.inst ? app.inst.escr() : app.buildDir().escr());
        }
        kul::env::CWD(this->project().dir());
    }
}

void maiken::Application::cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const throw(kul::Exception){
    if(par) par->cyclicCheck(apps);
    for(const auto& pa : apps)
        if(project().dir() == pa.first  && p == pa.second)
            KEXCEPTION("Cyclical dependency found\n"+project().dir().path());
}

void maiken::Application::addSourceLine(const std::string& o) throw (kul::StringException){
    if(o.find(',') == std::string::npos){
        for(const auto& s : kul::cli::asArgs(o)){
            kul::Dir d(resolveFromProperties(s));
            if(d) srcs.push_back(std::make_pair(d.real(), true));
            else{
                kul::File f(kul::Dir(resolveFromProperties(s)).locl());
                if(f) srcs.push_back(std::make_pair(f.real(), false));
                else  KEXCEPTION("source does not exist\n"+s+"\n"+project().dir().path());
            }
        }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXCEPTION("source invalid format\n" + project().dir().path());
        kul::Dir d(resolveFromProperties(v[0]));
        if(d) srcs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
        else KEXCEPTION("source does not exist\n"+v[0]+"\n"+project().dir().path());
    }
}
void maiken::Application::addIncludeLine(const std::string& o) throw (kul::StringException){
    if(o.find(',') == std::string::npos){
        for(const auto& s : kul::cli::asArgs(o))
            if(s.size()){
                kul::Dir d(resolveFromProperties(s));
                if(d) incs.push_back(std::make_pair(d.real(), true));
                else  KEXCEPTION("include does not exist\n"+d.path()+"\n"+project().dir().path());
            }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXCEPTION("include invalid format\n" + project().dir().path());
        kul::Dir d(resolveFromProperties(v[0]));
        if(d) incs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
        else  KEXCEPTION("include does not exist\n"+d.path()+"\n"+project().dir().path());
    }
}

void maiken::Application::loadTimeStamps() throw (kul::StringException){
    if(_MKN_TIMESTAMPS_){
        kul::Dir mkn(buildDir().join(".mkn"));
        kul::File src("src_stamp", mkn);
        if(mkn && src){
            kul::io::Reader r(src);
            const char* c = 0;
            while((c = r.readLine())){
                std::string s(c);
                if(s.size() == 0) continue;
                std::vector<std::string> bits;
                kul::String::SPLIT(s, ' ', bits);
                if(bits.size() != 2) KEXCEPTION("timestamp file invalid format\n"+src.full());
                try{
                    stss.insert(bits[0], kul::String::UINT16(bits[1]));
                }catch(const kul::StringException& e){
                    KEXCEPTION("timestamp file invalid format\n"+src.full());
                }
            }
        }
        kul::File inc("inc_stamp", mkn);
        if(mkn && inc){
            kul::io::Reader r(inc);
            const char* c = 0;
            while((c = r.readLine())){
                std::string s(c);
                if(s.size() == 0) continue;
                std::vector<std::string> bits;
                kul::String::SPLIT(s, ' ', bits);
                if(bits.size() != 2) KEXCEPTION("timestamp file invalid format\n"+inc.full());
                try{
                    itss.insert(bits[0], bits[1]);
                }catch(const kul::StringException& e){
                    KEXCEPTION("timestamp file invalid format\n"+src.full());
                }
            }
        }
        for(const auto& i : includes()){
            kul::Dir inc(i.first);
            ulonglong includeStamp = inc.timeStamps().modified();
            for(const auto fi : inc.files(1)) includeStamp += fi.timeStamps().modified();
            std::ostringstream os;
            os << std::hex << includeStamp;
            includeStamps.insert(inc.mini(), os.str());
        }
    }
}

void maiken::Application::setSuper(const Application* app){
    if(sup.get()) return;
    if(project().root()[SUPER]){
        const std::string& cwd(kul::env::CWD());
        kul::env::CWD(project().dir().real());
        kul::Dir d(project().root()[SUPER].Scalar());
        if(!d) KEXCEPTION("Super does not exist in project: " + project().dir().real());
        if(d.real() == project().dir().real())
            KEXCEPTION("Super cannot reference itself: " + project().dir().real());
        if(app && app->project().dir().real() == d.real())
            sup = std::make_shared<Application>(*app);
        else{
            sup = std::make_shared<Application>(maiken::Project::CREATE(d), "");
            kul::env::CWD(d.real());
            sup->setSuper(app ? app : this);
            sup->setup();
        }
        for(const auto& p : sup->properties()) if(!ps.count(p.first)) ps.insert(p.first, p.second);
        kul::env::CWD(cwd);
    }
}
