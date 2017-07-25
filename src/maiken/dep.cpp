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

void includeDependency(const std::string& s, const std::string& p, kul::hash::set::String& include){
    if(s == "+"){
        include.insert(s);
    }
    else{
        std::stringstream ss;
        ss << s << "[" << p << "]";
        include.insert(ss.str());
    }
}

void parseDepedencyString(std::string s, kul::hash::set::String& include){
    kul::String::REPLACE_ALL(s, " ", "");
    std::stringstream dep, pro;
    bool lB = 0, rB = 0;
    for(const auto& c : s){
        if(c == '[' && (lB || rB)) KEXIT(1, MKN_ERR_SQRBRKT_MISMATCH_DEP_CLI);
        else
        if(c == '[' && !lB) {
            if(dep.str().empty()) KEXIT(1, MKN_ERR_INVALID_DEP_CLI);
            lB = 1;
        }
        else
        if(c == ']'){
            if(pro.str().empty()) KEXIT(1, MKN_ERR_INVALID_DEP_CLI);
            lB = 0;
            rB = 1;
            includeDependency(dep.str(), pro.str(), include);
            pro.str(std::string());
            dep.str(std::string());
        }
        else
        if(c == ','){
            if(lB){
                includeDependency(dep.str(), pro.str(), include);
                pro.str(std::string());
            }
            else
            if(rB){
                rB = 0;
            }
            else{
                includeDependency(dep.str(), "@", include);
                dep.str(std::string());
            }
        }
        else{
            if(!lB) dep << c;
            else    pro << c;
        }
    }
    if(!rB) includeDependency(dep.str(), "@", include);
    if(lB) KEXIT(1, MKN_ERR_SQRBRKT_MISMATCH_DEP_CLI);
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
                parseDepedencyString(*depVal, include);
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
        const std::string& name(a.project().root()[STR_NAME].Scalar());
        std::stringstream ss;
        ss << name << "[" << (a.p.empty() ? "@" : a.p) << "]";
        if(AppVars::INSTANCE().dependencyLevel() || include.count(ss.str())) a.ig = 0;
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
        if(!all.count(d) && !ignore.count(d)) KEXIT(1, "Dependency project specified does not exist: "+ d);
    if(include.size() && include.count("+")) this->ig = 1;
}

void maiken::Application::buildDepVecRec(std::vector<Application*>& dePs, int16_t i, const kul::hash::set::String& inc){
    for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app){
        maiken::Application& a = (*app);
        const std::string& name(a.project().root()[STR_NAME].Scalar());
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

void maiken::Application::populateMapsFromDependencies() KTHROW(kul::Exception) {
    for(auto depP = dependencies().rbegin(); depP != dependencies().rend(); ++depP){
        const auto& dep(*depP);
        if(!dep.sources().empty()){
            const std::string n(dep.project().root()[STR_NAME].Scalar());
            const std::string lib = dep.baseLibFilename();
            const auto& it(std::find(libraries().begin(), libraries().end(), lib));
            if(it != libraries().end()) libs.erase(it);
            libs.push_back(lib);
        }

        for(const auto& s : dep.includes())
            if(s.second && std::find(includes().begin(), includes().end(), s) == includes().end())
                incs.push_back(std::make_pair(s.first, true));
        for(const std::string& s : dep.libraryPaths())
            if(std::find(libraryPaths().begin(), libraryPaths().end(), s) == libraryPaths().end())
                paths.push_back(s);
        for(const std::string& s : dep.libraries())
            if(std::find(libraries().begin(), libraries().end(), s) == libraries().end())
                libs.push_back(s);
    }
}