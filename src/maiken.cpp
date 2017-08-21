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
#include "maiken.hpp"

maiken::Application::Application(const maiken::Project& proj, const std::string& profile)
    : m(compiler::Mode::NONE), p(profile), proj(proj){}

maiken::Application::~Application(){
    for(auto mod: mods) mod->unload();
}

maiken::Application& maiken::Application::CREATE(int16_t argc, char *argv[]) KTHROW(kul::Exception){
    using namespace kul::cli;

    std::vector<Arg> argV {
        Arg('a', STR_ARG    , ArgType::STRING), Arg('A', STR_ADD  , ArgType::STRING),
        Arg('b', STR_BINC   , ArgType::STRING), Arg('B', STR_BPATH, ArgType::STRING),
        Arg('C', STR_DIR    , ArgType::STRING),
        Arg('d', STR_DEP    , ArgType::MAYBE) , Arg('D', STR_DEBUG),
        Arg('E', STR_ENV    , ArgType::STRING),
        Arg('f', STR_FINC   , ArgType::STRING), Arg('F', STR_FPATH, ArgType::STRING),
        Arg('G', STR_GET    , ArgType::STRING),
        Arg('h', STR_HELP),
        Arg('j', STR_JARG   , ArgType::STRING),
        Arg('K', STR_STATIC),
        Arg('l', STR_LINKER , ArgType::STRING), Arg('L', STR_ALINKER,  ArgType::STRING),
#ifndef _MKN_DISABLE_MODULES_
        Arg('m', STR_MOD,  ArgType::MAYBE),
#endif//_MKN_DISABLE_MODULES_
        Arg('M', STR_MAIN   , ArgType::MAYBE),
        Arg('o', STR_OUT    , ArgType::STRING),
        Arg('p', STR_PROFILE, ArgType::STRING), Arg('P', STR_PROPERTY, ArgType::STRING),
        Arg('r', STR_RUN_ARG, ArgType::STRING), Arg('R', STR_DRY_RUN),
        Arg('s', STR_SCM_STATUS), Arg('S', STR_SHARED),
        Arg('t', STR_THREADS, ArgType::MAYBE),
        Arg('u', STR_SCM_UPDATE), Arg('U', STR_SCM_FUPDATE),
        Arg('v', STR_VERSION),
        Arg('x', STR_SETTINGS,  ArgType::STRING)
    };
    std::vector<Cmd> cmdV {
        Cmd(STR_INIT),     Cmd(STR_INC),       Cmd(STR_SRC),
#ifndef _MKN_DISABLE_MODULES_
        Cmd(STR_MODS), Cmd(STR_BUILD_MOD),
#endif//_MKN_DISABLE_MODULES_
        Cmd(STR_CLEAN),    Cmd(STR_DEPS),
        Cmd(STR_BUILD),    Cmd(STR_BUILD_ALL), Cmd(STR_RUN),
        Cmd(STR_COMPILE),  Cmd(STR_LINK),      Cmd(STR_PROFILES),
        Cmd(STR_DBG),      Cmd(STR_PACK),      Cmd(STR_INFO),
        Cmd(STR_TRIM),     Cmd(STR_TREE)
    };
    Args args(cmdV, argV);
    try{
        args.process(argc, argv);
    }catch(const kul::cli::Exception& e){
        KEXIT(1, e.what());
    }
    if(args.empty() || (args.size() == 1 && args.has(STR_DIR))){
        if(args.size() == 1 && args.has(STR_DIR)){
            kul::Dir d(args.get(STR_DIR));
            if(!d) KEXIT(1, "STR_DIR DOES NOT EXIST: " + args.get(STR_DIR));
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
    if(args.empty() || args.has(STR_HELP)){
        showHelp();
        KEXIT(0, "");
    }
    if(args.has(STR_VERSION)){
        std::stringstream ss, mod;
        ss << KTOSTRING(_MKN_VERSION_) << " (" << KTOSTRING(__KUL_OS__) << ") w/[";
#ifndef _MKN_DISABLE_MODULES_
        mod << "mod,";
#endif//_MKN_DISABLE_MODULES_
        if(_MKN_TIMESTAMPS_) mod << "ts,";
        if(mod.str().size()) mod.seekp(-1, mod.cur);
        mod << "]";
        ss << mod.str();
        KOUT(NON) << ss.str();
        KEXIT(0, "");
    }
    if(args.has(STR_INIT)){
        NewProject p;
        KEXIT(0, "");
    }
    if(args.has(STR_DIR)) {
        kul::Dir d(args.get(STR_DIR));
        if(!d) KEXIT(1, "STR_DIR DOES NOT EXIST: " + args.get(STR_DIR));
        kul::env::CWD(args.get(STR_DIR));
    }
    const Project& project(*Projects::INSTANCE().getOrCreate(kul::env::CWD()));
    std::string profile;
    if(args.has(STR_PROFILE)){
        profile = args.get(STR_PROFILE);
        bool f = 0;
        for(const auto& n : project.root()[STR_PROFILE]){
            f = n[STR_NAME].Scalar() == profile;
            if(f) break;
        }
        if(!f) KEXIT(1, "profile does not exist");
    }
    if(args.has(STR_SETTINGS) && !Settings::SET(args.get(STR_SETTINGS)))
        KEXIT(1, "Unable to set specific settings xml");
    else Settings::INSTANCE();

    if(args.has(STR_DRY_RUN))       AppVars::INSTANCE().dryRun(true);
    if(args.has(STR_SHARED))        AppVars::INSTANCE().shar(true);
    if(args.has(STR_STATIC))        AppVars::INSTANCE().stat(true);
    if(AppVars::INSTANCE().shar() && AppVars::INSTANCE().stat())
        KEXIT(1, "Cannot specify shared and static simultaneously");
    if(args.has(STR_SCM_FUPDATE))   AppVars::INSTANCE().fupdate(true);
    if(args.has(STR_SCM_UPDATE))    AppVars::INSTANCE().update(true);
    if(args.has(STR_DEP))           AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());

    auto splitArgs = [](const std::string& s, const std::string& t, const std::function<void(const std::string&, const std::string&)>& f){
        for(const auto& p : kul::String::ESC_SPLIT(s, ',')){
            std::vector<std::string> ps = kul::String::ESC_SPLIT(p, '=');
            if(ps.size() > 2) KEXIT(1, t + " override invalid, escape extra \"=\"");
            f(ps[0], ps[1]);
        }
    };

    if(args.has(STR_PROPERTY))
        splitArgs(args.get(STR_PROPERTY),
            "property",
            std::bind(
                (void(AppVars::*)(const std::string&, const std::string&)) &AppVars::properkeys,
                std::ref(AppVars::INSTANCE()),
                std::placeholders::_1,
                std::placeholders::_2));
    if(args.has(STR_ENV))
        splitArgs(args.get(STR_ENV),
            "environment",
            std::bind(
                (void(AppVars::*)(const std::string&, const std::string&)) &AppVars::envVars,
                std::ref(AppVars::INSTANCE()),
                std::placeholders::_1,
                std::placeholders::_2));

    auto* app = Applications::INSTANCE().getOrCreate(project, profile);
    auto& a = *app;
    if(args.has(STR_PROFILES)){
        a.showProfiles();
        KEXIT(0, "");
    }
    a.ig = 0;

    if(args.has(STR_INFO)) a.showConfig(1);
    if(args.has(STR_TREE)) a.showTree();

    AppVars::INSTANCE().dependencyString(args.has(STR_DEP) ? &args.get(STR_DEP) : 0);
    a.buildDepVec(AppVars::INSTANCE().dependencyString());

    if(args.has(STR_GET)){
        const auto& get(args.get(STR_GET));
        if(a.properties().count(get)){
            KOUT(NON) << (*a.properties().find(get)).second;
        }
        if(AppVars::INSTANCE().properkeys().count(get)){
            KOUT(NON) << (*AppVars::INSTANCE().properkeys().find(get)).second;
        }
        KEXIT(0, "");
    }

    a.addCLIArgs(args);

    if(args.has(STR_MOD)){
        std::vector<std::string> vp { STR_COMPILE, STR_LINK, STR_PACK };
        if(args.get(STR_MOD).size()){
            for(const auto& s : kul::String::SPLIT(args.get(STR_MOD), ",")){
                if(std::find(vp.begin(), vp.end(), s) == vp.end())
                    KEXCEPSTREAM << "Invalid Module phase specified: " << s
                        << kul::os::EOL() << "Valid phases are: compile, link, pack";
                else if(AppVars::INSTANCE().modulePhases().count(s))
                    KEXCEPSTREAM << "Duplicate Module phase specified: " << s;
                else
                    AppVars::INSTANCE().modulePhase(s);
            }
        }else for(const auto& s : vp) AppVars::INSTANCE().modulePhase(s);
    }

    if(!args.has(STR_MOD) && !(args.has(STR_BUILD_MOD) || args.has(STR_BUILD_ALL))) a.modDeps.clear();

    if(args.has(STR_ARG))     AppVars::INSTANCE().args    (args.get(STR_ARG));
    if(args.has(STR_RUN_ARG)) AppVars::INSTANCE().runArgs (args.get(STR_RUN_ARG));
    if(args.has(STR_LINKER))  AppVars::INSTANCE().linker  (args.get(STR_LINKER));
    if(args.has(STR_ALINKER)) AppVars::INSTANCE().allinker(args.get(STR_ALINKER));
    if(args.has(STR_THREADS)){
        if(args.get(STR_THREADS).size())
            AppVars::INSTANCE().threads(kul::String::UINT16(args.get(STR_THREADS)));
        else AppVars::INSTANCE().threads(kul::cpu::threads());
    }
    if(args.has(STR_JARG)){
        try{
            YAML::Node node = YAML::Load(args.get(STR_JARG));
            for(YAML::const_iterator it = node.begin(); it != node.end(); ++it)
                for(const auto& s : kul::String::SPLIT(it->first.Scalar(), ':'))
                    AppVars::INSTANCE().jargs(s, it->second.Scalar());
        }catch(const std::exception& e){ KEXIT(1, "JSON args failed to parse"); }
    }

    auto printDeps = [&] (const std::vector<Application*>& vec) {
        std::vector<const Application*> v;
        for(auto app = vec.rbegin(); app != vec.rend(); ++app){
            const std::string& s((*app)->project().dir().real());
            auto it = std::find_if(v.begin(), v.end(), [&s](const Application* a) { return a->project().dir().real() == s;});
            if (it == v.end()) v.push_back(*app);
        }
        for(auto* app : v) KOUT(NON) <<  app->project().dir();
        KEXIT(0, "");
    };

    if(args.has(STR_DEPS)) printDeps(a.deps);
    if(args.has(STR_MODS)) printDeps(a.modDeps);

    if(args.has(STR_INC)){
        for(const auto& p : a.includes())
            KOUT(NON) << p.first;
        KEXIT(0, "");
    }
    if(args.has(STR_SRC)){
        for(const auto& p1 : a.sourceMap())
            for(const auto& p2 : p1.second)
                for(const auto& p3 : p2.second)
                    KOUT(NON) << kul::File(p3).full();
        for(auto app = a.deps.rbegin(); app != a.deps.rend(); ++app)
            for(const auto& p1 : (*app)->sourceMap())
                if(!(*app)->ig)
                    for(const auto& p2 : p1.second)
                        for(const auto& p3 : p2.second)
                            KOUT(NON) << kul::File(p3).full();
        KEXIT(0, "");
    }
    if(args.has(STR_SCM_STATUS)){
        a.scmStatus(args.has(STR_DEP));
        KEXIT(0, "");
    }

    if(args.has(STR_ADD))
        for(const auto& s : kul::String::ESC_SPLIT(args.get(STR_ADD), ','))
            a.addSourceLine(s);
    if(args.has(STR_MAIN)) a.main = args.get(STR_MAIN);
    if(args.has(STR_OUT))  a.out  = args.get(STR_OUT);

    if(args.has(STR_BUILD))     AppVars::INSTANCE().command(STR_BUILD);
    if(args.has(STR_BUILD_MOD)) AppVars::INSTANCE().command(STR_BUILD_MOD);
    if(args.has(STR_BUILD_ALL)) AppVars::INSTANCE().command(STR_BUILD_ALL);
    if(args.has(STR_CLEAN))     AppVars::INSTANCE().command(STR_CLEAN);
    if(args.has(STR_COMPILE))   AppVars::INSTANCE().command(STR_COMPILE);
    if(args.has(STR_LINK))      AppVars::INSTANCE().command(STR_LINK);
    if(args.has(STR_RUN))       AppVars::INSTANCE().command(STR_RUN);
    if(args.has(STR_DBG))       AppVars::INSTANCE().command(STR_DBG);
    if(args.has(STR_TRIM))      AppVars::INSTANCE().command(STR_TRIM);
    if(args.has(STR_PACK))      AppVars::INSTANCE().command(STR_PACK);

    return *app;
}

void maiken::Application::resolveLang() KTHROW(maiken::Exception) {
    const auto& mains(inactiveMains());
    if(mains.size()) lang = (*mains.begin()).substr((*mains.begin()).rfind(".")+1);
    else
    if(sources().size()){
        const auto srcMM = sourceMap();
        std::string maxS;
        kul::hash::map::S2T<size_t> mapS;
        size_t maxI = 0, maxO = 0;
        for(const auto& ft : srcMM) mapS.insert(ft.first, 0);
        for(const auto& ft : srcMM)
            mapS[ft.first] = mapS[ft.first] + ft.second.size();
        for(const auto& s_i : mapS)
            if(s_i.second > maxI){
                maxI = s_i.second;
                maxS = s_i.first;
            }
        for(const auto s_i : mapS) if(s_i.second == maxI) maxO++;
        if(maxO > 1)
            KEXCEPSTREAM
                << "file type conflict: linker filetype cannot be deduced, "
                << "specify lang tag to override\n"
                << project().dir().path();
        lang = maxS;
    }
}

kul::hash::set::String maiken::Application::inactiveMains() const{
    kul::hash::set::String iMs;
    std::string p;
    try{
        p = kul::Dir::REAL(main);
    }catch(const kul::Exception& e){ }
    std::string f;
    try{
        if(project().root()[STR_MAIN]){
            f = kul::Dir::REAL(project().root()[STR_MAIN].Scalar());
            if(p.compare(f) != 0) iMs.insert(f);
        }
    }catch(const kul::Exception& e){ }
    for(const YAML::Node& c : project().root()[STR_PROFILE]){
        try{
            if(c[STR_MAIN]){
                f = kul::Dir::REAL(c[STR_MAIN].Scalar());
                if(p.compare(f) != 0) iMs.insert(f);
            }
        }catch(const kul::Exception& e){ }
    }
    return iMs;
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
                s = l;
                while(s.size() && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')) s.pop_back();
                w << s.c_str();
            }
        }
    }
    f.rm();
    tmp.mv(f);
}

void maiken::Application::populateMaps(const YAML::Node& n) KTHROW(kul::Exception) { //IS EITHER ROOT OR PROFILE NODE!
    using namespace kul::cli;
    for(const auto& c : n[STR_ENV]){
        EnvVarMode mode = EnvVarMode::PREP;
        if(c[STR_MODE]){
            if      (c[STR_MODE].Scalar().compare(STR_APPEND)   == 0) mode = EnvVarMode::APPE;
            else if (c[STR_MODE].Scalar().compare(STR_PREPEND)  == 0) mode = EnvVarMode::PREP;
            else if (c[STR_MODE].Scalar().compare(STR_REPLACE)  == 0) mode = EnvVarMode::REPL;
            else KEXIT(1, "Unhandled EnvVar mode: " + c[STR_MODE].Scalar());
        }
        evs.erase(std::remove_if(evs.begin(), evs.end(),
            [&c](const EnvVar& ev) {return ev.name() == c[STR_NAME].Scalar();}), evs.end());
        evs.emplace_back(
            c[STR_NAME].Scalar(),
            Properties::RESOLVE(*this, c[STR_VALUE].Scalar()),
            mode);
    }
    for(const auto& p : AppVars::INSTANCE().envVars()){
        evs.erase(std::remove_if(evs.begin(), evs.end(),
            [&p](const EnvVar& ev) {return ev.name() == p.first;}), evs.end());
        evs.push_back(EnvVar(p.first, p.second, EnvVarMode::PREP));
    }

    if(n[STR_ARG])  for(const auto& o : kul::String::LINES(n[STR_ARG].Scalar()))  arg += Properties::RESOLVE(*this, o) + " ";
    if(n[STR_LINK]) for(const auto& o : kul::String::LINES(n[STR_LINK].Scalar())) lnk += Properties::RESOLVE(*this, o) + " ";
    try{
        if(n[STR_INC]) for(const auto& o : kul::String::LINES(n[STR_INC].Scalar())) addIncludeLine(o);
    }catch(const kul::StringException){
        KEXIT(1, "include contains invalid bool value\n"+project().dir().path());
    }
    try{
        if(n[STR_SRC]) for(const auto& o : kul::String::LINES(n[STR_SRC].Scalar())) addSourceLine(o);
    }catch(const kul::StringException){
        KEXIT(1, "source contains invalid bool value\n"+project().dir().path());
    }
    if(n[STR_PATH])
        for(const auto& s : kul::String::SPLIT(n[STR_PATH].Scalar(), ' '))
            if(s.size()){
                kul::Dir d(Properties::RESOLVE(*this, s));
                if(d) paths.push_back(d.escr());
                else KEXIT(1, "library path does not exist\n"+d.path()+"\n"+project().dir().path());
            }

    if(n[STR_LIB])
        for(const auto& s : kul::String::SPLIT(n[STR_LIB].Scalar(), ' '))
            if(s.size()) libs.push_back(Properties::RESOLVE(*this, s));

    for(const std::string& s : libraryPaths())
        if(!kul::Dir(s).is()) KEXIT(1, s + " is not a valid directory\n"+project().dir().path());
}

void maiken::Application::cyclicCheck(const std::vector<std::pair<std::string, std::string>>& apps) const KTHROW(kul::Exception){
    if(par) par->cyclicCheck(apps);
    for(const auto& pa : apps)
        if(project().dir() == pa.first  && p == pa.second)
            KEXIT(1, "Cyclical dependency found\n"+project().dir().path());
}

void maiken::Application::addIncludeLine(const std::string& o) KTHROW(kul::Exception){
    if(o.find(',') == std::string::npos){
        for(const auto& s : kul::cli::asArgs(o))
            if(s.size()){
                kul::Dir d(Properties::RESOLVE(*this, s));
                if(d) incs.push_back(std::make_pair(d.real(), true));
                else  KEXIT(1, "include does not exist\n"+d.path()+"\n"+project().dir().path());
            }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXIT(1, "include invalid format\n" + project().dir().path());
        kul::Dir d(Properties::RESOLVE(*this, v[0]));
        if(d) incs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
        else  KEXIT(1, "include does not exist\n"+d.path()+"\n"+project().dir().path());
    }
}

void maiken::Application::setSuper(){
    if(sup) return;
    if(project().root()[STR_SUPER]){
        const std::string& cwd(kul::env::CWD());
        kul::env::CWD(project().dir().real());
        kul::Dir d(project().root()[STR_SUPER].Scalar());
        if(!d) KEXIT(1, "Super does not exist in project: " + project().dir().real());
        std::string super(d.real());
        if(super == project().dir().real())
            KEXIT(1, "Super cannot reference itself: " + project().dir().real());
        d = kul::Dir(super);
        try{
            sup = Applications::INSTANCE().getOrCreate(*maiken::Projects::INSTANCE().getOrCreate(d), "");
            sup->resolveProperties();
        }catch(const std::exception& e){
            KEXIT(1, "Possible super cycle detected: " + project().dir().real());
        }
        auto cycle = sup;
        while(cycle){
            if(cycle->project().dir() == project().dir())
                KEXIT(1, "Super cycle detected: " + project().dir().real());
            cycle = cycle->sup;
        }
        for(const auto& p : sup->properties()) if(!ps.count(p.first)) ps.insert(p.first, p.second);
    }
    for(const auto& p : Settings::INSTANCE().properties())
        if(!ps.count(p.first)) ps.insert(p.first, p.second);
}