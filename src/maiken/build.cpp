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

void
maiken::Application::link(const kul::hash::set::String& objects) KTHROW(kul::Exception){
    showConfig();
    if(objects.size() > 0){
        buildDir().mk();
        if(!main.empty())   buildExecutable(objects);
        else                buildLibrary(objects);
    }else KEXIT(1, "No link objects found, try compile or build.");
}

void
maiken::Application::compile(kul::hash::set::String& objects) KTHROW(kul::Exception){
    showConfig();
    if(!AppVars::INSTANCE().dryRun()){
        std::stringstream ss;
        ss << MKN_PROJECT << ": " << this->project().dir().path();
        if(this->profile().size() > 0) ss << " [" << this->profile() << "]";
        KOUT(NON) << ss.str();
    }

    auto sources = sourceMap();
    for(const std::pair<std::string, kul::hash::map::S2T<kul::hash::set::String> >& ft : sources){
        try{
            if(!(*files().find(ft.first)).second.count(STR_COMPILER))
                KEXIT(1, "No compiler found for filetype " + ft.first);
            Compilers::INSTANCE().get((*(*files().find(ft.first)).second.find(STR_COMPILER)).second);
        }catch(const CompilerNotFoundException& e){
            KEXIT(1, e.what());
        }
    }

    if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() && this->includes().size()){
        KOUT(NON) << "INCLUDES";
        for(const auto& s : this->includes()) KOUT(NON) << "\t" << s.first;
    }
    if(!AppVars::INSTANCE().dryRun() && srcs.empty() && main.empty()){
        KOUT(NON) << "NO SOURCES";
        return;
    }
    buildDir().mk();
    std::vector<kul::File> cacheFiles;
    if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf()){
        if(!arg.empty()) KOUT(NON) << "ARGUMENTS\n\t" << arg;
        if(this->arguments().size()){
            KOUT(NON) << "FILE ARGUMENTS";
            for(const auto& kv : this->arguments())
                for(const std::string& s : kv.second) KOUT(NON) << "\t" << kv.first << " : " << s;
        }
        if(!AppVars::INSTANCE().args().empty())
            KOUT(NON) << "ADDITIONAL ARGUMENTS: \n\t" << AppVars::INSTANCE().args();
        if(AppVars::INSTANCE().jargs().size()){
            KOUT(NON) << "ADDITIONAL FILE ARGUMENTS:";
            for(const auto& kv : AppVars::INSTANCE().jargs())
                KOUT(NON) << "\t" << kv.first << " : " << kv.second;
        }
    }
    if(!AppVars::INSTANCE().envVars().count("MKN_OBJ")) KEXCEPTION("INTERNAL BADNESS ERROR!");
    auto o = [] (const std::string& s) { if(s.size()) KOUT(NON) << s; };
    auto e = [] (const std::string& s) { if(s.size()) KERR << s; };
    const std::string oType("." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
    for(const std::pair<std::string, kul::hash::map::S2T<kul::hash::set::String> >& ft : sources){
        kul::Dir objD(buildDir().join("obj")); objD.mk();
        const Compiler* compiler = Compilers::INSTANCE().get((*(*files().find(ft.first)).second.find(STR_COMPILER)).second);
        if(compiler->sourceIsBin()){
            for(const std::pair<std::string, kul::hash::set::String>& kv : ft.second)
                for(const std::string& s : kv.second){
                    kul::File source(s);
                    objects.insert(source.escm());
                    cacheFiles.push_back(source);
                }
        }else{
            std::queue<std::pair<std::string, std::string> > sourceQueue;
            for(const std::pair<std::string, kul::hash::set::String>& kv : ft.second){
                for(const std::string& s : kv.second){
                    const kul::File source(s);
                    if(!incSrc(source)) continue;
                    std::stringstream ss, os;
                    ss << std::hex << std::hash<std::string>()(source.real());
                    os << ss.str() << "-" << source.name() << oType;
                    kul::File object(os.str(), objD);
                    sourceQueue.push(std::pair<std::string, std::string>(
                        AppVars::INSTANCE().dryRun() ? source.esc() : source.escm(),
                        AppVars::INSTANCE().dryRun() ? object.esc() : object.escm()));
                }
            }

            ThreadingCompiler tc(*this);
            kul::ConcurrentThreadPool<> ctp(AppVars::INSTANCE().threads(), 1);
            auto lambda = [&](const std::pair<std::string, std::string>& pair){
                if(ctp.exception()) ctp.interrupt();
                const CompilerProcessCapture cpc = tc.compile(pair);
                if(!AppVars::INSTANCE().dryRun()){
                    if(kul::LogMan::INSTANCE().inf() || cpc.exception()) o(cpc.outs());
                    if(kul::LogMan::INSTANCE().inf() || cpc.exception()) e(cpc.errs());
                    KOUT(INF) << cpc.cmd();
                }else KOUT(NON) << cpc.cmd();
                if(cpc.exception()) std::rethrow_exception(cpc.exception());
            };
            auto lambex = [&](const kul::Exception& e){
                ctp.stop();
                throw e;
            };

            std::queue<std::pair<std::string, std::string> > cQueue;
            while(sourceQueue.size() > 0){
                ctp.async(std::bind(lambda, sourceQueue.front()), std::bind(lambex, std::placeholders::_1));
                cQueue.push(sourceQueue.front());
                sourceQueue.pop();
            }
            ctp.finish(1000000 * 1000);
            if(ctp.exception()) KEXIT(1, "Compile error detected");

            while(cQueue.size()){
                objects.insert(cQueue.front().second);
                cacheFiles.push_back(kul::File(cQueue.front().first));
                cQueue.pop();
            }
        }
    }
    if(_MKN_TIMESTAMPS_) writeTimeStamps(objects, cacheFiles);
}

maiken::CompilerProcessCapture
maiken::Application::buildExecutable(const kul::hash::set::String& objects){
    using namespace kul;
    const std::string& file     = main;
    const std::string& fileType = file.substr(file.rfind(".") + 1);
    if(fs.count(fileType) > 0){
        if(!(*files().find(fileType)).second.count(STR_COMPILER)) KEXIT(1, "No compiler found for filetype " + fileType);
        if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() && !this->libraries().empty()){
            KOUT(NON) << "LIBRARIES";
            for(const std::string& s : this->libraries()) KOUT(NON) << "\t" << s;
        }
        if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() && !this->libraryPaths().empty()){
            KOUT(NON) << "LIBRARY PATHS";
            for(const std::string& s : this->libraryPaths()) KOUT(NON) << "\t" << s;
        }
        try{
            std::string linker = fs[fileType][STR_LINKER];
            std::string linkEnd = AppVars::INSTANCE().linker();
            if(!AppVars::INSTANCE().allinker().empty()) linkEnd += " " + AppVars::INSTANCE().allinker();
            if(!lnk.empty()) linkEnd += " " + lnk;
            kul::Dir outD(inst ? inst.real() : buildDir());
            if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() && linkEnd.size())
                KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
            const std::string& n(out.empty() ? project().root()[STR_NAME].Scalar() : out);
            std::string bin(AppVars::INSTANCE().dryRun() ? kul::File(outD.join(n)).esc() : kul::File(outD.join(n)).escm());
            std::vector<std::string> obV;
            for(const auto& o : objects) obV.emplace_back(o);
            const CompilerProcessCapture& cpc =
                Compilers::INSTANCE().get((*(*files().find(fileType)).second.find(STR_COMPILER)).second)
                    ->buildExecutable(linker, linkEnd, obV,
                        libraries(), libraryPaths(), bin, m, AppVars::INSTANCE().dryRun());
            if(AppVars::INSTANCE().dryRun()) KOUT(NON) << cpc.cmd();
            else{
                checkErrors(cpc);
                KOUT(INF) << cpc.cmd();
                KOUT(NON) << "Creating bin: " << kul::File(cpc.file()).real();
            }
            return cpc;
        }catch(const CompilerNotFoundException& e){
            KEXCEPTION("UNSUPPORTED COMPILER EXCEPTION");
        }
    }else
        KEXIT(1, "Unable to handle artifact: \"" + file + "\" - type is not in file list");
}

maiken::CompilerProcessCapture
maiken::Application::buildLibrary(const kul::hash::set::String& objects){
    if(fs.count(lang) > 0){
        if(m == compiler::Mode::NONE) m = compiler::Mode::SHAR;
        if(!(*files().find(lang)).second.count(STR_COMPILER)) KEXIT(1, "No compiler found for filetype " + lang);
        std::string linker = fs[lang][STR_LINKER];
        std::string linkEnd;
        if(!par) linkEnd = AppVars::INSTANCE().linker();
        if(!AppVars::INSTANCE().allinker().empty()) linkEnd += " " + AppVars::INSTANCE().allinker();
        if(!lnk.empty()) linkEnd += " " + lnk;
        if(!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() && linkEnd.size())
            KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
        if(m == compiler::Mode::STAT) linker = fs[lang][STR_ARCHIVER];
        kul::Dir outD(inst ? inst.real() : buildDir());
        std::string lib(baseLibFilename());
        lib = AppVars::INSTANCE().dryRun() ? kul::File(lib, outD).esc() : kul::File(lib, outD).escm();
        std::vector<std::string> obV;
        for(const auto& o : objects) obV.emplace_back(o);
        const CompilerProcessCapture& cpc =
            Compilers::INSTANCE().get((*(*files().find(lang)).second.find(STR_COMPILER)).second)
                ->buildLibrary(linker, linkEnd, obV,
                    libraries(), libraryPaths(), lib, m, AppVars::INSTANCE().dryRun());
        if(AppVars::INSTANCE().dryRun()) KOUT(NON) << cpc.cmd();
        else{
            checkErrors(cpc);
            KOUT(INF) << cpc.cmd();
            KOUT(NON) << "Creating lib: " << kul::File(cpc.file()).real();
        }
        return cpc;
    }else
        KEXCEPTION("Unable to handle artifact: \"" + lang + "\" - type is not in file list");
}

void
maiken::Application::checkErrors(const CompilerProcessCapture& cpc) KTHROW(kul::Exception){
    auto o = [] (const std::string& s) { if(s.size()) KOUT(NON) << s; };
    auto e = [] (const std::string& s) { if(s.size()) KERR << s; };
    if(kul::LogMan::INSTANCE().inf() || cpc.exception())
        o(cpc.outs());
    if(kul::LogMan::INSTANCE().err() || cpc.exception())
        e(cpc.errs());
    if(cpc.exception()) std::rethrow_exception(cpc.exception());
}