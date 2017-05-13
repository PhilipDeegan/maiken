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
#include "maiken.hpp"

void maiken::Application::showConfig(bool force){
    if(AppVars::INSTANCE().show() || AppVars::INSTANCE().dryRun()) return;
    if(kul::LogMan::INSTANCE().inf() || force){
        std::string repo = (*AppVars::INSTANCE().properkeys().find("MKN_REPO")).second;
        using namespace kul::cli;
        KOUT(NON) << "+++++++++ BUILD INFO ++++++++";
        KOUT(NON) << "REPO    : " << repo;
        KOUT(NON) << "THREADS : " << AppVars::INSTANCE().threads() << "\n";
        KOUT(NON) << "BINARIES";
        std::string path = kul::env::GET("PATH");
        for(const YAML::Node& c : Settings::INSTANCE().root()[STR_ENV]){
            if(c[STR_NAME].Scalar() != "PATH") continue;
            EnvVarMode mode = EnvVarMode::APPE;
            if      (c[STR_MODE].Scalar().compare(STR_APPEND)   == 0) mode = EnvVarMode::APPE;
            else if (c[STR_MODE].Scalar().compare(STR_PREPEND)  == 0) mode = EnvVarMode::PREP;
            else if (c[STR_MODE].Scalar().compare(STR_REPLACE)  == 0) mode = EnvVarMode::REPL;
            else KEXCEPT(Exception, "Unhandled EnvVar mode: " + c[STR_MODE].Scalar());
            path = EnvVar(c[STR_NAME].Scalar(), c[STR_VALUE].Scalar(), mode).toString();
            break;
        }
        {
            auto it(std::find_if(evs.begin(), evs.end(), [](const kul::cli::EnvVar& ev) { return strcmp(ev.name(), "PATH") == 0;}));
            if(it != evs.end()) path = (*it).toString();
        }
        for(const auto& c : Settings::INSTANCE().root()[STR_FILE]){
            bool a = 0, g = 0, l = 0;
            KOUT(NON) << "TYPE    : " << c[STR_TYPE].Scalar();
            std::vector<std::string> ps;
            kul::String::SPLIT(path, kul::env::SEP(), ps);
            for(const auto& d : ps){
                if(a && g && l) break;
                kul::Dir dir(d);
                if(!dir) continue;
                for(const auto& f : dir.files()){
                    std::string b = (f.name().size() > 3 && f.name().substr(f.name().size() - 4) == ".exe") ?
                        f.name().substr(0, f.name().size() - 4) : f.name();
                    if(!a && c[STR_ARCHIVER] && b == kul::String::SPLIT(c[STR_ARCHIVER].Scalar(), " ")[0]){
                        KOUT(NON) << "ARCHIVER: " << f.full();
                        a = 1;
                        break;
                    }
                }
                for(const auto& f : dir.files()){
                    std::string b = (f.name().size() > 3 && f.name().substr(f.name().size() - 4) == ".exe") ?
                        f.name().substr(0, f.name().size() - 4) : f.name();
                    if(!g && c[STR_COMPILER])
                        for(const auto& k : kul::code::Compilers::INSTANCE().keys())
                            if(b == k) {
                                KOUT(NON) << "COMPILER: " << f.full();
                                g = 1;
                                break;
                            }
                }
                for(const auto& f : dir.files()){
                    std::string b = (f.name().size() > 3 && f.name().substr(f.name().size() - 4) == ".exe") ?
                        f.name().substr(0, f.name().size() - 4) : f.name();
                    if(!l && c[STR_LINKER] && b == kul::String::SPLIT(c[STR_LINKER].Scalar(), " ")[0]){
                        KOUT(NON) << "LINKER  : " << f.full();
                        l = 1;
                        break;
                    }
                }
            }
        }
        KOUT(NON) << "+++++++++++++++++++++++++++++";
    }
    AppVars::INSTANCE().show(1);
}

void maiken::Application::showHelp(){
    std::vector<std::string> ss;
    ss.push_back(MKN_DEFS_CMD);
    ss.push_back(MKN_DEFS_BUILD);
    ss.push_back(MKN_DEFS_BUILD_ALL);
    ss.push_back(MKN_DEFS_BUILD_MOD);
    ss.push_back(MKN_DEFS_CLEAN);
    ss.push_back(MKN_DEFS_COMP);
    ss.push_back(MKN_DEFS_DBG);
    ss.push_back(MKN_DEFS_INIT);
    ss.push_back(MKN_DEFS_LINK);
    ss.push_back(MKN_DEFS_PACK);
    ss.push_back(MKN_DEFS_PROFS);
    ss.push_back(MKN_DEFS_RUN);
    ss.push_back(MKN_DEFS_INC);
    ss.push_back(MKN_DEFS_SRC);
    ss.push_back(MKN_DEFS_TRIM);
    ss.push_back("");
    ss.push_back(MKN_DEFS_ARG);
    ss.push_back(MKN_DEFS_ARGS);
    ss.push_back(MKN_DEFS_ADD);    
    ss.push_back(MKN_DEFS_DIRC);
    ss.push_back(MKN_DEFS_DEPS);
    ss.push_back(MKN_DEFS_EVSA);
    ss.push_back(MKN_DEFS_GET);
    ss.push_back(MKN_DEFS_HELP);
    ss.push_back(MKN_DEFS_JARG);
    ss.push_back(MKN_DEFS_OUT);
    ss.push_back(MKN_DEFS_MOD);
    ss.push_back(MKN_DEFS_MAIN);
    ss.push_back(MKN_DEFS_STATIC);
    ss.push_back(MKN_DEFS_LINKER);
    ss.push_back(MKN_DEFS_ALINKR);
    ss.push_back(MKN_DEFS_PROF);
    ss.push_back(MKN_DEFS_PROP);
    ss.push_back(MKN_DEFS_RUN_ARGS);
    ss.push_back(MKN_DEFS_DRYR);
    ss.push_back(MKN_DEFS_STAT);
    ss.push_back(MKN_DEFS_SHARED);
    ss.push_back(MKN_DEFS_THREDS);
    ss.push_back(MKN_DEFS_UPDATE);
    ss.push_back(MKN_DEFS_FUPDATE);
    ss.push_back(MKN_DEFS_VERSON);
    ss.push_back(MKN_DEFS_SETTNGS);
    ss.push_back("");
    ss.push_back(MKN_DEFS_EXMPL);
    ss.push_back(MKN_DEFS_EXMPL1);
    ss.push_back(MKN_DEFS_EXMPL2);
    ss.push_back(MKN_DEFS_EXMPL3);
    ss.push_back(MKN_DEFS_EXMPL4);
    ss.push_back("");
    for(const auto& s : ss) KOUT(NON) << s;
}

void maiken::Application::showProfiles(){
    std::vector<std::string> ss;
    uint b = 0, o = 0;
    for(const auto& n : this->project().root()[STR_PROFILE]){
        b = n[STR_NAME].Scalar().size() > b ? n[STR_NAME].Scalar().size() : b;
        o = n["os"] ? n["os"].Scalar().size() > o ? n["os"].Scalar().size() : o : o;
    }
    for(const auto& n : this->project().root()[STR_PROFILE]){
        std::string s(n[STR_NAME].Scalar());
        kul::String::PAD(s, b);
        std::string os(n["os"] ? "("+n["os"].Scalar()+")" : "");
        if(!os.empty()) kul::String::PAD(os, o);
        std::stringstream s1;
        s1 << "\t" << s << os;
        if(n[STR_PARENT]) s1 << "\t" << MKN_PARENT << ": " << Properties::RESOLVE(*this, n[STR_PARENT].Scalar());
        ss.push_back(s1.str());
    }
    KOUT(NON) << MKN_PROFILE;
    for(const auto& s : ss) KOUT(NON) << s;
}