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

void maiken::Application::populateMapsFromModules(){
    // for(auto mod = modDeps.rbegin(); mod != modDeps.rend(); ++mod){
    //     for(const std::string& s : (*mod).libraryPaths())
    //         if(std::find(libraryPaths().begin(), libraryPaths().end(), s) == libraryPaths().end())
    //             paths.push_back(s);
    //     for(const std::string& s : (*mod).libraries())
    //         if(std::find(libraries().begin(), libraries().end(), s) == libraries().end())
    //             libs.push_back(s);
    // }
}

kul::File maiken::ModuleLoader::FIND(const Application& ap)
#ifndef _MKN_DISABLE_MODULES_
        throw(kul::sys::Exception)
#endif//_MKN_DISABLE_MODULES_
{
#ifdef _MKN_DISABLE_MODULES_
    KEXCEPT(kul::Exception, "Modules are disabled: ") << ap.project().dir();
#endif//_MKN_DISABLE_MODULES_

    std::string file;
    for(const auto& f : ap.buildDir().files(0)){
        const auto& name(f.name());
        if(name.find(".") != std::string::npos
            && name.find(ap.project().root()["name"].Scalar()) != std::string::npos
#ifdef _WIN32
            && name.substr(name.rfind(".") + 1) == "dll"){
#else
            && name.substr(name.rfind(".") + 1) == "so"){
#endif
            file = ap.buildDir().join(name);
            break;
        }
    }

    kul::File lib(file);
#ifndef _MKN_DISABLE_MODULES_
    if(!lib) KEXCEPT(kul::sys::Exception, "No loadable library found for project: ") << ap.project().dir();
#endif//_MKN_DISABLE_MODULES_
    return lib;
}

std::shared_ptr<maiken::ModuleLoader> maiken::ModuleLoader::LOAD(const Application& ap)
#ifndef _MKN_DISABLE_MODULES_
        throw(kul::sys::Exception)
#endif//_MKN_DISABLE_MODULES_
{
    for(auto dep = ap.dependencies().rbegin(); dep != ap.dependencies().rend(); ++dep)
        if(!(*dep).sources().empty()) GlobalModules::INSTANCE().load(*dep);
    return std::make_shared<ModuleLoader>(ap, kul::File(FIND(ap)));
}