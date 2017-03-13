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

class ModuleMinimiser{
    friend class maiken::Application;
    private:
        void add(const std::vector<maiken::Application>& mods, kul::hash::map::S2T<maiken::Application>& apps){
            for(const auto& m : mods)
                if(!apps.count(m.buildDir().real()))
                    apps.insert(m.buildDir().real(), m);
        }
    public:
        static ModuleMinimiser& INSTANCE(){
            static ModuleMinimiser a;
            return a;
        }
        kul::hash::map::S2T<maiken::Application> modules(maiken::Application& app){
            kul::hash::map::S2T<maiken::Application> apps;
            add(app.moduleDependencies(), apps);
            for(auto dep = app.dependencies().rbegin(); dep != app.dependencies().rend(); ++dep)
                add(dep->moduleDependencies(), apps);
            return apps;
        }
};

maiken::Application::Application(const maiken::Project& proj, const std::string& profile)
    : m(kul::code::Mode::NONE), p(profile), proj(proj){}

maiken::Application::~Application(){
    for(auto mod: mods) mod->unload();
}

maiken::Application& maiken::Application::CREATE(int16_t argc, char *argv[]) throw(kul::Exception){
    using namespace kul::cli;

#ifdef _MKN_DISABLE_MODULES_
    KOUT(NON) << "Warning: module functionality disabled";
#endif//_MKN_DISABLE_MODULES_

    std::vector<Arg> argV { Arg('a', STR_ARG    ,  ArgType::STRING), Arg('A', STR_ADD, ArgType::STRING),
                            Arg('C', STR_DIR    , ArgType::STRING),
                            Arg('d', STR_DEP    ,  ArgType::MAYBE), Arg('D', STR_DEBUG),
                            Arg('E', STR_ENV    ,  ArgType::STRING),
                            Arg('h', STR_HELP),
                            Arg('j', STR_JARG   ,  ArgType::STRING),
                            Arg('K', STR_STATIC),
                            Arg('l', STR_LINKER ,  ArgType::STRING), Arg('L', STR_ALINKER,  ArgType::STRING),
#ifndef _MKN_DISABLE_MODULES_
                            Arg('m', STR_MOD,  ArgType::MAYBE),
#endif//_MKN_DISABLE_MODULES_
                            Arg('M', STR_MAIN, ArgType::MAYBE),
                            Arg('o', STR_OUT,      ArgType::STRING),
                            Arg('p', STR_PROFILE,  ArgType::STRING), Arg('P', STR_PROPERTY, ArgType::STRING),
                            Arg('R', STR_DRY_RUN),
                            Arg('s', STR_SCM_STATUS), Arg('S', STR_SHARED),
                            Arg('t', STR_THREADS, ArgType::MAYBE),
                            Arg('u', STR_SCM_UPDATE), Arg('U', STR_SCM_FUPDATE),
                            Arg('v', STR_VERSION),
                            Arg('x', STR_SETTINGS,  ArgType::STRING)};
    std::vector<Cmd> cmdV { Cmd(STR_INIT),     Cmd(STR_INC),       Cmd(STR_SRC),
#ifndef _MKN_DISABLE_MODULES_
                            Cmd(STR_MODS), Cmd(STR_BUILD_MOD),
#endif//_MKN_DISABLE_MODULES_
                            Cmd(STR_CLEAN),    Cmd(STR_DEPS),
                            Cmd(STR_BUILD),    Cmd(STR_BUILD_ALL), Cmd(STR_RUN),
                            Cmd(STR_COMPILE),  Cmd(STR_LINK),      Cmd(STR_PROFILES),
                            Cmd(STR_DBG),      Cmd(STR_PACK),      Cmd(STR_TRIM),
                            Cmd(STR_INFO)};
    Args args(cmdV, argV);
    try{
        args.process(argc, argv);
    }catch(const kul::cli::Exception& e){
        showHelp();
        KEXIT(0, "");
    }
    if(args.empty() || (args.size() == 1 && args.has(STR_DIR))){
        if(args.size() == 1 && args.has(STR_DIR)){
            kul::Dir d(args.get(STR_DIR));
            if(!d) KEXCEPTION("STR_DIR DOES NOT EXIST: " + args.get(STR_DIR));
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
        KOUT(NON) << KTOSTRING(_MKN_VERSION_) << " (" << KTOSTRING(__KUL_OS__) << ")";
        KEXIT(0, "");
    }
    if(args.has(STR_INIT)){
        NewProject p;
        KEXIT(0, "");
    }
    if(args.has(STR_DIR)) {
        kul::Dir d(args.get(STR_DIR));
        if(!d) KEXCEPTION("STR_DIR DOES NOT EXIST: " + args.get(STR_DIR));
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
        if(!f) KEXCEPT(Exception, "profile does not exist");
    }
    if(args.has(STR_SETTINGS) && !Settings::SET(args.get(STR_SETTINGS)))
        KEXCEPT(Exception, "Unable to set specific settings xml");
    else Settings::INSTANCE();
    auto* app = Applications::INSTANCE().getOrCreate(project, profile);
    auto& a = *app;
    if(args.has(STR_PROFILES)){
        a.showProfiles();
        KEXIT(0, "");
    }
    a.ig = 0;
    if(args.has(STR_DRY_RUN))       AppVars::INSTANCE().dryRun(true);
    if(args.has(STR_SHARED))        AppVars::INSTANCE().shar(true);
    if(args.has(STR_STATIC))        AppVars::INSTANCE().stat(true);
    if(AppVars::INSTANCE().shar() && AppVars::INSTANCE().stat())
        KEXCEPT(Exception, "Cannot specify shared and static simultaneously");
    if(args.has(STR_SCM_FUPDATE))   AppVars::INSTANCE().fupdate(true);
    if(args.has(STR_SCM_UPDATE))    AppVars::INSTANCE().update(true);

    if(project.root()[STR_SCM])     a.scr = project.root()[STR_SCM].Scalar();

    if(args.has(STR_DEP)) AppVars::INSTANCE().dependencyLevel((std::numeric_limits<int16_t>::max)());

    auto splitArgs = [](const std::string& s, const std::string& t, const std::function<void(const std::string&, const std::string&)>& f){
        for(const auto& p : kul::String::ESC_SPLIT(s, ',')){
            std::vector<std::string> ps = kul::String::ESC_SPLIT(p, '=');
            if(ps.size() > 2) KEXCEPTION(t + " override invalid, escape extra \"=\"");
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

    AppVars::INSTANCE().dependencyString(args.has(STR_DEP) ? &args.get(STR_DEP) : 0);

    a.setup();
    a.buildDepVec(AppVars::INSTANCE().dependencyString());

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
        }catch(const std::exception& e){ KEXCEPTION("JSON args failed to parse"); }
    }

    auto printDeps = [&] (const std::vector<Application>& vec) {
        std::vector<const Application*> v;
        for(auto app = vec.rbegin(); app != vec.rend(); ++app){
            const std::string& s((*app).project().dir().real());
            auto it = std::find_if(v.begin(), v.end(), [&s](const Application* app) { return (*app).project().dir().real() == s;});
            if (it == v.end()) v.push_back(&(*app));
        }
        for(auto* app : v) KOUT(NON) <<  (*app).project().dir();
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
            for(const auto& p1 : (*app).sourceMap())
                if(!(*app).ig)
                    for(const auto& p2 : p1.second)
                        for(const auto& p3 : p2.second)
                            KOUT(NON) << kul::File(p3).full();
        KEXIT(0, "");
    }
    if(args.has(STR_SCM_STATUS)){
        a.scmStatus(args.has(STR_DEP));
        KEXIT(0, "");
    }
    if(args.has(STR_INFO)) a.showConfig(1);

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


class CommandStateMachine{
    friend class maiken::Application;
    private:
        bool _main = 1;
        kul::hash::set::String cmds;
        CommandStateMachine(){
            reset();
        }
        static CommandStateMachine& INSTANCE(){
            static CommandStateMachine a;
            return a;
        }
        void reset(){
            cmds.clear();
            for(const auto& s : maiken::AppVars::INSTANCE().commands())
                cmds.insert(s);
        }
        void add(const std::string& s){
            cmds.insert(s);
        }
        const kul::hash::set::String& commands(){
            return cmds;
        }
        void main(bool m){
            _main = m;
        }
        bool main(){
            return _main;
        }
};

class BuildRecorder{
    friend class maiken::Application;
    private:
        kul::hash::set::String builds;
        static BuildRecorder& INSTANCE(){
            static BuildRecorder a;
            return a;
        }
        void add(const std::string& k){
            builds.insert(k);
        }
        bool has(const std::string& k){
            return builds.count(k);
        }
};

void maiken::Application::process() throw(kul::Exception){
    const kul::hash::set::String& cmds (CommandStateMachine::INSTANCE().commands());
    const kul::hash::set::String& phase(AppVars::INSTANCE().modulePhases());

    auto loadModules = [&] (Application& app) {
#ifndef _MKN_DISABLE_MODULES_
        if(phase.size())
            for(auto mod = app.modDeps.begin(); mod != app.modDeps.end(); ++mod)
                app.mods.push_back(ModuleLoader::LOAD(*mod));
#endif//_MKN_DISABLE_MODULES_
    };
#ifdef _MKN_DISABLE_MODULES_
    if(modDeps.size()) KOUT(ERR) << "Modules disabled in binary";
#endif//_MKN_DISABLE_MODULES_

    auto proc = [&] (Application& app, bool work) {
        kul::env::CWD(app.project().dir());

        if(work){
            if(!app.buildDir()) app.buildDir().mk();
            if(BuildRecorder::INSTANCE().has(app.buildDir().real())) return;
            BuildRecorder::INSTANCE().add(app.buildDir().real());
        }

        kul::Dir mkn(app.buildDir().join(".mkn"));
        std::vector<std::pair<std::string, std::string> > oldEvs;
        for(const kul::cli::EnvVar& ev : app.envVars()){
            const std::string v = kul::env::GET(ev.name());
            oldEvs.push_back(std::pair<std::string, std::string>(ev.name(), v));
            kul::env::SET(ev.name(), ev.toString().c_str());
        }
        if(cmds.count(STR_CLEAN) && app.buildDir().is()){
            app.buildDir().rm();
            mkn.rm();
        }
        app.loadTimeStamps();
        if(cmds.count(STR_TRIM)) app.trim();

        std::vector<std::string> objects;
        if(cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD) || cmds.count(STR_COMPILE)){
            if(phase.count(STR_COMPILE))
                for(auto& modLoader : app.mods)
                    modLoader->module()->compile(app, modLoader->app()->modCArg);
            if(work) app.compile(objects);
        }
        if(cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD) || cmds.count(STR_LINK)){
            if(phase.count(STR_LINK))
                for(auto& modLoader : app.mods)
                    modLoader->module()->link(app, modLoader->app()->modLArg);
            if(work) objects.empty() ? app.link() : app.link(objects);
        }
        for(const std::pair<std::string, std::string>& oldEv : oldEvs)
            kul::env::SET(oldEv.first.c_str(), oldEv.second.c_str());
    };

    if(cmds.count(STR_BUILD_ALL) || cmds.count(STR_BUILD_MOD)){
        auto _mods = ModuleMinimiser::INSTANCE().modules(*this);
        if(_mods.size() && !cmds.count(STR_BUILD_ALL)) CommandStateMachine::INSTANCE().add(STR_BUILD);
        CommandStateMachine::INSTANCE().main(0);
        for(auto& m : _mods) m.second.process();
        CommandStateMachine::INSTANCE().main(1);
    }

    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app)
        if(!(*app).ig) loadModules(*app);
    if(!this->ig) loadModules(*this);

    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app){
        if((*app).ig) continue;
        proc(*app, !(*app).srcs.empty());
    }
    if(!this->ig) proc(*this, (!this->srcs.empty() || !this->main.empty()));

    if(cmds.count(STR_PACK)){
        pack();
        if(phase.count(STR_PACK))
            for(auto& modLoader : mods)
                modLoader->module()->pack(*this, modLoader->app()->modPArg);
    }
    if(CommandStateMachine::INSTANCE().main() && (cmds.count(STR_RUN) || cmds.count(STR_DBG))) run(cmds.count(STR_DBG));
    CommandStateMachine::INSTANCE().reset();
}

void maiken::Application::setup(){
    if(AppVars::INSTANCE().update() || AppVars::INSTANCE().fupdate()) {
        scmUpdate(AppVars::INSTANCE().fupdate());
        Projects::INSTANCE().reload(proj);
    }
    setSuper();
    if(scr.empty()) scr = project().root()[STR_NAME].Scalar();

    this->resolveProperties();
    this->preSetupValidation();
    std::string buildD = kul::Dir::JOIN(STR_BIN, p);
    if(p.empty()) buildD = kul::Dir::JOIN(STR_BIN, STR_BUILD);
    this->bd = kul::Dir(project().dir().join(buildD));
    std::string profile(p);
    std::vector<YAML::Node> nodes;
    if(profile.empty()){
        nodes.push_back(project().root());
        profile = project().root()[STR_NAME].Scalar();
    }
    if(project().root()[STR_PROFILE])
        for (std::size_t i=0;i < project().root()[STR_PROFILE].size(); i++)
            nodes.push_back(project().root()[STR_PROFILE][i]);

    using namespace kul::cli;
    for(const YAML::Node& c : Settings::INSTANCE().root()[STR_ENV]){
        EnvVarMode mode = EnvVarMode::PREP;
        if      (c[STR_MODE].Scalar().compare(STR_APPEND)   == 0) mode = EnvVarMode::APPE;
        else if (c[STR_MODE].Scalar().compare(STR_PREPEND)  == 0) mode = EnvVarMode::PREP;
        else if (c[STR_MODE].Scalar().compare(STR_REPLACE)  == 0) mode = EnvVarMode::REPL;
        evs.emplace_back(
            c[STR_NAME].Scalar(),
            Properties::RESOLVE(*this, c[STR_VALUE].Scalar()),
            mode);
    }

    auto getIfMissing = [&](const YAML::Node& n, const bool mod){
        const std::string& cwd(kul::env::CWD());
        kul::Dir projectDir(resolveDepOrModDirectory(n, mod));
        if(!projectDir.is()) loadDepOrMod(n, projectDir, mod);
        kul::env::CWD(cwd);
    };

    bool c = 1;
    while(c){
        c = 0;
        for (const auto& n : nodes) {
            if(n[STR_NAME].Scalar() != profile) continue;
            for(const auto& mod : n[STR_MOD]) getIfMissing(mod, 1);
            popDepOrMod(n, modDeps, STR_MOD, 1);
            if(n[STR_IF_MOD] && n[STR_IF_MOD][KTOSTRING(__KUL_OS__)]) {
                for(const auto& mod : n[STR_IF_MOD][KTOSTRING(__KUL_OS__)]) 
                    getIfMissing(mod, 1);
                popDepOrMod(n[STR_IF_MOD], modDeps, KTOSTRING(__KUL_OS__), 1);
            }
            profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }

    auto depLevel(AppVars::INSTANCE().dependencyLevel());
    for(auto& mod : modDeps) {
        mod.ig = 0;
        mod.buildDepVec(AppVars::INSTANCE().dependencyString());
    }
    AppVars::INSTANCE().dependencyLevel(depLevel);

    c = 1;
    profile = p.size() ? p : project().root()[STR_NAME].Scalar();
    while(c){
        c = 0;
        for (const auto& n : nodes) {
            if(n[STR_NAME].Scalar() != profile) continue;
            for(const auto& dep : n[STR_DEP]) getIfMissing(dep, 0);
            populateMaps(n);
            popDepOrMod(n, deps, STR_DEP, 0);
            if(n[STR_IF_DEP] && n[STR_IF_DEP][KTOSTRING(__KUL_OS__)]){
                for(const auto& dep : n[STR_IF_DEP][KTOSTRING(__KUL_OS__)])
                    getIfMissing(dep, 0);
                popDepOrMod(n[STR_IF_DEP], deps, KTOSTRING(__KUL_OS__), 0);
            }
            profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }

    if(Settings::INSTANCE().root()[STR_INC])
        for(const auto& l : kul::String::LINES(Settings::INSTANCE().root()[STR_INC].Scalar()))
            for(const auto& s : kul::cli::asArgs(l))
                if(s.size()){
                    kul::Dir d(Properties::RESOLVE(*this, s));
                    if(d) incs.push_back(std::make_pair(d.real(), false));
                    else  KEXCEPTION("include does not exist\n")
                        << d.path() << "\n"
                        << Settings::INSTANCE().file();
                }
    if(Settings::INSTANCE().root()[STR_PATH])
        for(const auto& l : kul::String::LINES(Settings::INSTANCE().root()[STR_PATH].Scalar()))
            for(const auto& s : kul::cli::asArgs(l))
                if(s.size()){
                    kul::Dir d(Properties::RESOLVE(*this, s));
                    if(d) paths.push_back(d.escr());
                    else
                        KEXCEPTION("library path does not exist\n")
                            << d.path() << "\n"
                            << Settings::INSTANCE().file();
                }

    this->populateMapsFromDependencies();
    std::vector<std::string> fileStrings{STR_ARCHIVER, STR_COMPILER, STR_LINKER};
    for(const auto& c : Settings::INSTANCE().root()[STR_FILE])
        for(const std::string& s : fileStrings)
            for(const auto& t : kul::String::SPLIT(c[STR_TYPE].Scalar(), ':'))
                if(fs[t].count(s) == 0 && c[s])
                    fs[t].insert(s, Properties::RESOLVE(*this, c[s].Scalar()));

    this->postSetupValidation();
    profile = p.size() ? p : project().root()[STR_NAME].Scalar();
    bool nm = 1;
    c = 1;
    while(c){
        c = 0;
        for(const auto& n : nodes){
            if(n[STR_NAME].Scalar() != profile) continue;
            if(n[STR_MODE] && nm){
                m = n[STR_MODE].Scalar() == STR_STATIC ? kul::code::Mode::STAT
                : n[STR_MODE].Scalar() == STR_SHARED ? kul::code::Mode::SHAR
                : kul::code::Mode::NONE;
                nm = 0;
            }
            if(out.empty()  && n[STR_OUT])  out  = Properties::RESOLVE(*this, n[STR_OUT].Scalar());
            if(main.empty() && n[STR_MAIN]) main = n[STR_MAIN].Scalar();
            if(lang.empty() && n[STR_LANG]) lang = n[STR_LANG].Scalar();
            profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
    }
    if(main.empty() && lang.empty()){
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
    if(par){
        if(!main.empty() && lang.empty()) lang = main.substr(main.rfind(".")+1);
        main.clear();
    }
    if(nm){
        if(AppVars::INSTANCE().shar()) m = kul::code::Mode::SHAR;
        else
        if(AppVars::INSTANCE().stat()) m = kul::code::Mode::STAT;
    }
    profile = p.size() ? p : project().root()[STR_NAME].Scalar();
    c = 1;
    while(c){
        c = 0;
        const auto& propK = AppVars::INSTANCE().properkeys();
        for(const auto& n : nodes){
            if(n[STR_NAME].Scalar() != profile) continue;
            if(inst.path().empty()){
                if(Settings::INSTANCE().root()[STR_LOCAL]
                    && propK.count("MKN_BIN")
                    && !main.empty())
                    inst = kul::Dir((*propK.find("MKN_BIN")).second);
                else
                if(Settings::INSTANCE().root()[STR_LOCAL]
                    && propK.count("MKN_LIB")
                    && main.empty())
                    inst = kul::Dir((*propK.find("MKN_LIB")).second);
                else
                if(n[STR_INSTALL]) inst = kul::Dir(Properties::RESOLVE(*this, n[STR_INSTALL].Scalar()));
                if(!inst.path().empty()){
                    if(!inst && !inst.mk())
                        KEXCEPTION("install tag is not a valid directory\n" + project().dir().path());
                    inst = kul::Dir(inst.real());
                }
            }
            if(n[STR_IF_ARG])
                for(YAML::const_iterator it = n[STR_IF_ARG].begin(); it != n[STR_IF_ARG].end(); ++it){
                    std::string left(it->first.Scalar());
                    if(left.find("_") != std::string::npos){
                        if(left.substr(0, left.find("_")) == KTOSTRING(__KUL_OS__))
                            left = left.substr(left.find("_") + 1);
                        else continue;
                    }
                    std::vector<std::string> ifArgs;
                    for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                        ifArgs.push_back(Properties::RESOLVE(*this, s));
                    if(lang.empty() && left == STR_BIN) for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(main.empty() && left == STR_LIB) for(const auto& s : ifArgs) arg += s + " ";
                    if(m == kul::code::Mode::SHAR && left == STR_SHARED)
                        for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(m == kul::code::Mode::STAT && left == STR_STATIC)
                        for(const auto& s : ifArgs) arg += s + " ";
                    else
                    if(left == KTOSTRING(__KUL_OS__)) for(const auto& s : ifArgs) arg += s + " ";
                }
            try{
                if(n[STR_IF_INC])
                    for(YAML::const_iterator it = n[STR_IF_INC].begin(); it != n[STR_IF_INC].end(); ++it)
                        if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                            for(const auto& s : kul::String::LINES(it->second.Scalar()))
                                addIncludeLine(s);
            }catch(const kul::StringException& e){
                KLOG(ERR) << e.what();
                KEXCEPTION("if_inc contains invalid bool value\n"+project().dir().path());
            }
            try{
                if(n[STR_IF_SRC])
                    for(YAML::const_iterator it = n[STR_IF_SRC].begin(); it != n[STR_IF_SRC].end(); ++it)
                        if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                            for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                                addSourceLine(s);
            }catch(const kul::StringException){
                KEXCEPTION("if_src contains invalid bool value\n"+project().dir().path());
            }
            if(n[STR_IF_LIB])
                for(YAML::const_iterator it = n[STR_IF_LIB].begin(); it != n[STR_IF_LIB].end(); ++it)
                    if(it->first.Scalar() == KTOSTRING(__KUL_OS__))
                        for(const auto& s : kul::String::SPLIT(it->second.Scalar(), ' '))
                            if(s.size()) libs.push_back(Properties::RESOLVE(*this, s));

            profile = n[STR_PARENT] ? Properties::RESOLVE(*this, n[STR_PARENT].Scalar()) : "";
            c = !profile.empty();
            break;
        }
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

void maiken::Application::run(bool dbg){
    std::string bin(out.empty() ? project().root()[STR_NAME].Scalar() : out);
#ifdef _WIN32
    bin += ".exe";
#endif
    kul::File f(bin, inst ? inst : buildDir());
    if(!f) KEXCEPTION("binary does not exist \n" + f.full());
    std::unique_ptr<kul::Process> p;
    if(dbg){
        std::string dbg = kul::env::GET("MKN_DBG");
        if(dbg.empty())
            if(Settings::INSTANCE().root()[STR_LOCAL] && Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER])
                dbg = Settings::INSTANCE().root()[STR_LOCAL][STR_DEBUGGER].Scalar();
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
                s = l;
                while(s.size() && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')) s.pop_back();
                w << s.c_str();
            }
        }
    }
    f.rm();
    tmp.mv(f);
}

void maiken::Application::populateMaps(const YAML::Node& n){ //IS EITHER ROOT OR PROFILE NODE!
    using namespace kul::cli;
    for(const auto& c : n[STR_ENV]){
        EnvVarMode mode = EnvVarMode::PREP;
        if      (c[STR_MODE].Scalar().compare(STR_APPEND)   == 0) mode = EnvVarMode::APPE;
        else if (c[STR_MODE].Scalar().compare(STR_PREPEND)  == 0) mode = EnvVarMode::PREP;
        else if (c[STR_MODE].Scalar().compare(STR_REPLACE)  == 0) mode = EnvVarMode::REPL;
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
        KEXCEPT(Exception, "include contains invalid bool value\n"+project().dir().path());
    }
    try{
        if(n[STR_SRC]) for(const auto& o : kul::String::LINES(n[STR_SRC].Scalar())) addSourceLine(o);
    }catch(const kul::StringException){
        KEXCEPT(Exception, "source contains invalid bool value\n"+project().dir().path());
    }
    if(n[STR_PATH])
        for(const auto& s : kul::String::SPLIT(n[STR_PATH].Scalar(), ' '))
            if(s.size()){
                kul::Dir d(Properties::RESOLVE(*this, s));
                if(d) paths.push_back(d.escr());
                else KEXCEPTION("library path does not exist\n"+d.path()+"\n"+project().dir().path());
            }

    if(n[STR_LIB])
        for(const auto& s : kul::String::SPLIT(n[STR_LIB].Scalar(), ' '))
            if(s.size()) libs.push_back(Properties::RESOLVE(*this, s));

    for(const std::string& s : libraryPaths())
        if(!kul::Dir(s).is()) KEXCEPTION(s + " is not a valid directory\n"+project().dir().path());
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
            kul::Dir d(Properties::RESOLVE(*this, s));
            if(d) srcs.push_back(std::make_pair(d.real(), true));
            else{
                kul::File f(kul::Dir(Properties::RESOLVE(*this, s)).locl());
                if(f) srcs.push_back(std::make_pair(f.real(), false));
                else  KEXCEPTION("source does not exist\n"+s+"\n"+project().dir().path());
            }
        }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXCEPTION("source invalid format\n" + project().dir().path());
        kul::Dir d(Properties::RESOLVE(*this, v[0]));
        if(d) srcs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
        else KEXCEPTION("source does not exist\n"+v[0]+"\n"+project().dir().path());
    }
}
void maiken::Application::addIncludeLine(const std::string& o) throw (kul::StringException){
    if(o.find(',') == std::string::npos){
        for(const auto& s : kul::cli::asArgs(o))
            if(s.size()){
                kul::Dir d(Properties::RESOLVE(*this, s));
                if(d) incs.push_back(std::make_pair(d.real(), true));
                else  KEXCEPTION("include does not exist\n"+d.path()+"\n"+project().dir().path());
            }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXCEPTION("include invalid format\n" + project().dir().path());
        kul::Dir d(Properties::RESOLVE(*this, v[0]));
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

namespace maiken {
class SuperApplications{
    friend class maiken::Application;
    private:
        kul::hash::set::String files;
        static SuperApplications& INSTANCE(){
            static SuperApplications instance;
            return instance;
        }
        void cycleCheck(const std::string& file) throw (maiken::SettingsException){
            if(files.count(file))
                KEXCEPT(maiken::SettingsException, "Super cycle detected in file: " + file);
            files.insert(file);
        }
};
}

void maiken::Application::setSuper(){
    if(sup) return;
    if(project().root()[STR_SUPER]){
        const std::string& cwd(kul::env::CWD());
        kul::env::CWD(project().dir().real());
        kul::Dir d(project().root()[STR_SUPER].Scalar());
        if(!d) KEXCEPTION("Super does not exist in project: " + project().dir().real());
        std::string super(d.real());
        if(super == project().dir().real())
            KEXCEPTION("Super cannot reference itself: " + project().dir().real());
        SuperApplications::INSTANCE().cycleCheck(super);
        d = kul::Dir(super);
        sup = Applications::INSTANCE().getOrCreate(
            *maiken::Projects::INSTANCE().getOrCreate(d), "");
        for(const auto& p : sup->properties()) if(!ps.count(p.first)) ps.insert(p.first, p.second);
        kul::env::CWD(cwd);
    }
    for(const auto& p : Settings::INSTANCE().properties())
        if(!ps.count(p.first)) ps.insert(p.first, p.second);
}