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

void maiken::Application::resolveProperties(){
    ps.setDeletedKey("PLEASE");
    for(YAML::const_iterator it=project().root()[PROPERTY].begin();it!=project().root()[PROPERTY].end(); ++it)
        ps[it->first.as<std::string>()] = it->second.as<std::string>();
    for(YAML::const_iterator it=project().root()[PROPERTY].begin();it!=project().root()[PROPERTY].end(); ++it) {
        std::string s = this->resolveFromProperties(it->second.as<std::string>());
        if(ps.count(it->first.as<std::string>())) ps.erase(it->first.as<std::string>());
        ps[it->first.as<std::string>()] = s;
    }
}

std::string maiken::Application::resolveFromProperties(const std::string& s) const{
    std::string r = s;
    int lb  = s.find("${");
    int clb = s.find("\\${");
    int rb  = s.find("}");
    int crb = s.find("\\}");
    while((lb - clb + 1) == 0){
        lb  = r.substr(clb + 3).find("${");
        clb = r.substr(clb + 3).find("\\${");
    }
    while((rb - crb + 1) == 0){
        rb  = r.substr(crb + 2).find("}");
        crb = r.substr(crb + 2).find("\\}");
    }
    if(lb != -1 && clb == -1 && rb != -1 && crb == -1){
        std::string k(r.substr(lb + 2, rb - 2 - lb));
        if(ps.count(k) == 0){
            if(AppVars::INSTANCE().properkeys().count(k))
                k = (*AppVars::INSTANCE().properkeys().find(k)).second;
            else KEXCEPT(Exception, "Property : '" + k + "' has not been defined");
        }else k = (*ps.find(k)).second;
        r = resolveFromProperties(r.substr(0, lb) + k + r.substr(rb + 1));
    }
    kul::String::TRIM(r);
    return r;
}


