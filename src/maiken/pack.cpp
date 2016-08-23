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

void maiken::Application::pack() throw(kul::Exception){
    kul::Dir pk(buildDir().join("pack"));
    if(!pk && !pk.mk()) KEXCEPTION("Cannot create: " + pk.path());

    kul::Dir bin(pk.join("bin"), main.size());
    kul::Dir lib(pk.join("lib"));
    
    const auto& binar(buildDir().files(0));
    if((!main.empty() || srcs.empty()) && binar.size() == 0)
        KEXCEPTION("No files found, try building");

    for(const auto& f : binar) f.cp(main.size() ? bin : lib);

    kul::hash::set::String libS;

    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app)
        if(!(*app).srcs.empty()){ 
            const auto& libar((*app).buildDir().files(0));
            if(libar.size() == 0) KEXCEPTION("No files found, try building, " + (*app).project().dir().real());
            for(const auto& f : libar) {
                if(f.name().find(".") == std::string::npos) continue;
                f.cp(lib);
                libS.insert(f.name().substr(0, f.name().rfind(".")));
            }
        }
    for(const auto& l : libs){
        bool f1 = 0;
        for(const auto& p : paths){
            kul::Dir path(p);
            if(!path) KEXCEPTION("Path does not exist: " + pk.path());
            for(const auto& f : path.files(0)){
                if(f.name().find(".") == std::string::npos) continue;
                if(libS.count(f.name().substr(0, f.name().rfind(".")))) {
                    f1 = 1;
                    break;
                }
#ifdef  _WIN32
                if(f.name().substr(0, f.name().rfind(".")) == l){
#else
                if(f.name().size() > 3 && f.name().substr(0, 3) == "lib" 
                    && f.name().substr(3, f.name().rfind(".") - 3) == l){
#endif//_WIN32
                    f.cp(lib);
                    f1 = 1;
                    break;
                }
            }
            if(f1) break;
        }
        if(!f1) KOUT(NON) << "Could not find library, assumed system based for: " << l;
    }
}
