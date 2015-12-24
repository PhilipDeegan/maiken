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

void maiken::Application::build() throw(kul::Exception){
    showConfig();
    const std::vector<std::string>& objects = this->compile();
    if(objects.size() > 0){
        buildDir().mk();
        if(!main.empty())   buildExecutable(objects);
        else                buildLibrary(objects);
    }
}

void maiken::Application::link() throw(kul::Exception){
    showConfig();
    std::vector<std::string> objects;
    for(const auto& ft : sourceMap()){
        try{
            if(!(*files().find(ft.first)).second.count(COMPILER))
                KEXCEPTION("No compiler found for filetype " + ft.first);
            const auto* compiler = kul::code::Compilers::INSTANCE().get((*(*files().find(ft.first)).second.find(COMPILER)).second);
            if(!compiler->sourceIsBin()){
                if(!buildDir().is()) KEXCEPT(maiken::Exception, "Cannot link without compiling.");
                for(const kul::File f : buildDir().files(true)){
                    if(f.name().substr(f.name().rfind(".") + 1).compare("o") == 0)
                        objects.push_back(f.real());
                }
            }else{
                for(const auto& kv : ft.second)
                    for(const auto& f : kv.second) objects.push_back(kul::File(f).mini());
            }
        }catch(const kul::code::CompilerNotFoundException& e){
            KEXCEPTION("No compiler found for filetype " + ft.first);
        }
    }
    if(objects.size() > 0){
        buildDir().mk();
        if(!main.empty())   buildExecutable(objects);
        else                buildLibrary(objects);
    }else KEXCEPTION("No objects found, try \"compile\" first.");
}

const std::vector<std::string> maiken::Application::compile() throw(kul::Exception){
    showConfig();
    std::vector<std::string> objects;
    KOUT(NON);
    KOUT(NON) << MKN_PROJECT << ": " << this->project().dir().path();
    if(this->profile().size() > 0) KOUT(NON) << MKN_PROFILE << ": " << this->profile();
    if(kul::LogMan::INSTANCE().inf() && this->includes().size()){
        KOUT(NON) << "INCLUDES";
        for(const auto& s : this->includes()) KOUT(NON) << "\t" << s.first;
    }
    auto& sources = sourceMap();
    if(sources.size() == 0){
        KOUT(NON) << "NO SOURCES";
        return objects;
    }
    buildDir().mk();
    if(kul::LogMan::INSTANCE().inf()){
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
    for(const std::pair<std::string, kul::hash::map::S2T<kul::hash::set::String> >& ft : sources){
        const kul::code::Compiler* compiler;
        try{
            if(!(*files().find(ft.first)).second.count(COMPILER))
                KEXCEPT(Exception, "No compiler found for filetype " + ft.first);
            compiler = kul::code::Compilers::INSTANCE().get((*(*files().find(ft.first)).second.find(COMPILER)).second);
        }catch(const kul::code::CompilerNotFoundException& e){
            KEXCEPT(Exception, "No compiler found for filetype " + ft.first);
        }
        if(compiler->sourceIsBin()){
            for(const std::pair<std::string, kul::hash::set::String>& kv : ft.second)
                for(const std::string& src : kv.second)
                    objects.push_back(kul::File(src).mini());
        }else{
            std::queue<std::pair<std::string, std::string> > sourceQueue;
            for(const std::pair<std::string, kul::hash::set::String>& kv : ft.second){
                for(const std::string& src : kv.second){
                    const kul::Dir d = kul::File(src).dir();
                    if(d.path().size() == kv.first.size())
                        kul::Dir(buildDir().join("obj"), true);
                    else
                        kul::Dir(kul::Dir::JOIN(buildDir().join("obj"), d.path().substr(kv.first.size())), true);
                    std::string obj(kul::Dir::JOIN(buildDir().join("obj"), src.substr(kv.first.size() + 1)));
                    if(obj.find(kul::env::CWD()) != std::string::npos)
                        obj = obj.substr(kul::env::CWD().size() + 1) + ".obj";
                    sourceQueue.push(std::pair<std::string, std::string>(kul::File(src).mini(), obj));
                }
            }
            while(sourceQueue.size() > 0){
                std::queue<std::pair<std::string, std::string> > tQueue;
                for(unsigned int i = 0; i < AppVars::INSTANCE().threads() && sourceQueue.size() > 0; i++){
                    tQueue.push(sourceQueue.front());
                    objects.push_back(sourceQueue.front().second);
                    sourceQueue.pop();
                }
                if(tQueue.size() == 0 && sourceQueue.size() == 0) break;
                ThreadingCompiler tc(*this, tQueue);
                kul::Ref<ThreadingCompiler> ref(tc);
                kul::PredicatedThreadPool<std::queue<std::pair<std::string, std::string> > > tp(ref, tQueue);
                tp.setMax(AppVars::INSTANCE().threads());
                tp.run();
                tp.join();

                auto f = [] (const std::string& s) { if(s.size()) KOUT(NON) << s; };
                std::exception_ptr ep;
                for(const kul::code::CompilerProcessCapture& cpc : tc.processCaptures()){
                    if(cpc.exception()) ep = cpc.exception();
                    if(cpc.exception()){
                        f(cpc.outs());
                        f(cpc.errs());
                    }else
                    if(kul::LogMan::INSTANCE().dbg()) f(cpc.cmd());
                    else
                    if(kul::LogMan::INSTANCE().inf()){
                        f(cpc.outs());
                        f(cpc.errs());
                    }
                }
                if(ep) std::rethrow_exception(ep);
            }
        }
    }
    return objects;
}

const kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String> > maiken::Application::sourceMap(){
    const kul::hash::set::String iMs = inactiveMains();
    kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> sm;
    if(main.size()){
        kul::File f(main);
        if(!f)      f = kul::File(main, project().dir());
        if(f.is())  sm[f.name().substr(f.name().rfind(".") + 1)][f.dir().real()].insert(f.real());
        else        KOUT(NON)  << "WARN : main does not exist: " << f;
    }
    for(const std::pair<std::string, bool>& sourceDir : sources()){
        for(const kul::File& file : kul::Dir(sourceDir.first).files(sourceDir.second)){
            if(file.name().find(".") == std::string::npos || file.name().substr(0, 1).compare(".") == 0) continue;
            const std::string ft = file.name().substr(file.name().rfind(".") + 1);
            if(fs.count(ft) > 0){
                const std::string& rl(file.real());
                bool a = false;
                for(const std::string& s : iMs){
                    a = rl.compare(s) == 0;
                    if(a) break;
                }
                if(!a) sm[ft][sourceDir.first].insert(rl);
            }
        }
    }
    return sm;
}

void maiken::Application::buildExecutable(const std::vector<std::string>& objects){
    using namespace kul;

    const std::string& file     = main;
    const std::string& fileType = file.substr(file.rfind(".") + 1);
    if(fs.count(fileType) > 0){
        if(!(*files().find(fileType)).second.count(COMPILER)) KEXCEPT(Exception, "No compiler found for filetype " + fileType);
        if(kul::LogMan::INSTANCE().inf() && !this->libraries().empty()){
            KOUT(NON) << "LIBRARIES";
            for(const std::string& s : this->libraries()) KOUT(NON) << "\t" << s;
        }
        if(kul::LogMan::INSTANCE().inf() && !this->libraryPaths().empty()){
            KOUT(NON) << "LIBRARY PATHS";
            for(const std::string& s : this->libraryPaths()) KOUT(NON) << "\t" << s;
        }
        try{
            std::string linker = fs[fileType][LINKER];
            std::string linkEnd = AppVars::INSTANCE().linker();
            kul::Dir out(inst ? inst.real() : buildDir());
            if(linkEnd.size()) KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
            const std::string& n(project().root()[NAME].Scalar());
            const kul::code::CompilerProcessCapture& cpc =
                kul::code::Compilers::INSTANCE().get((*(*files().find(fileType)).second.find(COMPILER)).second)
                    ->buildExecutable(linker, linkEnd, objects, 
                        libraries(), libraryPaths(), out.join(n), m);
            checkErrors(cpc);
            kul::Dir mkn(out.join(".mkn"));
            if(!mkn.is() && !mkn.mk()) KEXCEPTION("Inadequate access for directory: " +out.path());
            if(!kul::File("built", mkn).mk()) KEXCEPTION("Inadequate access for directory: " +out.path());
            KOUT(DBG) << cpc.cmd();
            KOUT(NON) << "Creating bin: " << cpc.tmp();
        }catch(const kul::code::CompilerNotFoundException& e){
            KEXCEPTION("UNSUPPORTED COMPILER EXCEPTION");
        }
    }else
        KEXCEPTION("Unable to handle artifact: \"" + file + "\" - type is not in file list");
}

void maiken::Application::buildLibrary(const std::vector<std::string>& objects){
    if(fs.count(lang) > 0){
        if(!(*files().find(lang)).second.count(COMPILER)) KEXCEPT(Exception, "No compiler found for filetype " + lang);

        std::string linker = fs[lang][LINKER];
        std::string linkEnd;
        if(!par){
            linkEnd = AppVars::INSTANCE().linker();
            if(linkEnd.size()) KOUT(NON) << "LINKER ARGUMENTS\n\t" << linkEnd;
        }
        if(m == kul::code::Mode::STAT) linker = fs[lang][ARCHIVER];

        const std::string& n(project().root()[NAME].Scalar());
        kul::Dir out(inst ? inst.real() : buildDir());
        std::string lib(inst ? p.empty() ? n : n+"_"+p : n);
        const kul::code::CompilerProcessCapture& cpc =
            kul::code::Compilers::INSTANCE().get((*(*files().find(lang)).second.find(COMPILER)).second)
                ->buildLibrary(linker, linkEnd, objects, 
                    libraries(), libraryPaths(), kul::File(lib, out), m);
        checkErrors(cpc);
        kul::Dir mkn(out.join(".mkn"));
        if(!mkn.is() && !mkn.mk()) KEXCEPTION("Inadequate access for directory: " +out.path());
        if(!kul::File("built", mkn).mk()) KEXCEPTION("Inadequate access for directory: " +out.path());
        KOUT(DBG) << cpc.cmd();
        KOUT(NON) << "Creating lib: " << cpc.tmp();

    }else
        KEXCEPTION("Unable to handle artifact: \"" + lang + "\" - type is not in file list");
}

void maiken::Application::checkErrors(const kul::code::CompilerProcessCapture& cpc) throw(kul::Exception){
    if(kul::LogMan::INSTANCE().inf()){
        auto f = [] (const std::string& s) { if(s.size()) KOUT(NON) << s; };
        f(cpc.outs());
        f(cpc.errs());
    }
    if(cpc.exception()) std::rethrow_exception(cpc.exception());
}

