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
// #include "kul/log.hpp"

#include "maiken.hpp"

class Validator : public maiken::Constants{
    private:
        std::vector<std::string> ifArgsLefts   { "bin", "lib", "shared", "static"};
        std::vector<std::string> ifIncSrcLefts { "bsd", "nix", "win"};
        Validator(){
            std::vector<std::string> ifLefts;
            for(const std::string& is : ifIncSrcLefts) for(const std::string& a : ifArgsLefts) ifLefts.push_back(is + "_" + a);
            for(const std::string& s : ifLefts) ifArgsLefts.push_back(s);
            for(const std::string& s : ifIncSrcLefts) ifArgsLefts.push_back(s);
        }
        static Validator& INSTANCE(){
            static Validator v;
            return v;
        }
        static void IF_VALUEDATER(const maiken::Application& a, const YAML::Node& n, const std::string& s, const std::vector<std::string>& lefts){
            kul::hash::set::String keys;
            for(YAML::const_iterator it= n.begin(); it != n.end(); ++it){
                if(std::find(lefts.begin(), lefts.end(), it->first.Scalar()) == lefts.end())
                    KEXCEPT(maiken::Exception, "malformed "+s+" key, \n"+a.project().dir().path());
                if(keys.count(it->first.Scalar())) KEXCEPT(maiken::Exception, "Duplicate "+s+"key detected: "+it->first.Scalar()+"\n"+a.project().dir().path());
                keys.insert(it->first.Scalar());
            }
        }
    public:
        static void PRE_BUILD(const maiken::Application& a, const YAML::Node& n) throw (maiken::Exception){
            if(n[MAIN] && n[LANG])
                KEXCEPT(maiken::Exception, "cannot have both main and lang tag\n"+a.project().dir().path());
            if(n[MAIN]){
                const std::string& m(n[MAIN].Scalar());
                if(m.find(".") == std::string::npos)
                    KEXCEPT(maiken::Exception, "main tag invalid format, expects <file>.<type>\n"+a.project().dir().path());
            }
            if(n[MODE]){
                const auto& s(n[MODE].Scalar());
                if(s != NONE && s != STATIC && s != SHARED)
                    KEXCEPT(maiken::Exception, "mode tag invalid value, expects none/static/shared\n"+a.project().dir().path());
            }
            if(n[IF_ARG]) IF_VALUEDATER(a, n[IF_ARG], IF_ARG, INSTANCE().ifArgsLefts);
            if(n[IF_INC]) IF_VALUEDATER(a, n[IF_INC], IF_ARG, INSTANCE().ifIncSrcLefts);
            if(n[IF_SRC]) IF_VALUEDATER(a, n[IF_SRC], IF_ARG, INSTANCE().ifIncSrcLefts);
            if(n[IF_LIB]) IF_VALUEDATER(a, n[IF_SRC], IF_ARG, INSTANCE().ifIncSrcLefts);
        }
        static void POST_BUILD(const maiken::Application& a, const YAML::Node& n) throw (maiken::Exception){
            std::stringstream ss;
            for(const auto f : a.files()) ss << f.first << " ";
            if(n[MAIN] && !a.files().count(n[MAIN].Scalar().substr(n[MAIN].Scalar().rfind(".")+1)))
                KEXCEPT(maiken::Exception, "main tag invalid type, valid types are\n"+ss.str()+"\n"+a.project().dir().path());
            else
            if(n[LANG] && !a.files().count(n[LANG].Scalar()))
                KEXCEPT(maiken::Exception, "lang tag invalid type, valid types are\n"+ss.str()+"\n"+a.project().dir().path());
        }
};

void maiken::Application::preSetupValidation() throw (maiken::Exception){
    {
        kul::hash::set::String keys;
        for(YAML::const_iterator it=project().root()[PROPERTY].begin();it!=project().root()[PROPERTY].end(); ++it){
            if(keys.count(it->first.Scalar())) KEXCEPTION("Duplicate PROPERTIES: "+it->first.Scalar()+"\n"+project().dir().path());
            keys.insert(it->first.Scalar());
        }
    }
    bool dpp = project().root()[PARENT];
    bool dpf = 0;
    Validator::PRE_BUILD(*this, project().root());
    std::vector<std::string> profiles;
    for(const auto& profile : project().root()[PROFILE]){
        const std::string& p(profile[NAME].Scalar());
        if(p == project().root()[NAME].Scalar())
            KEXCEPTION("Profile may not have same name as project");
        if(std::find(profiles.begin(), profiles.end(), p) != profiles.end())
            KEXCEPTION("Duplicate profile name found");
        profiles.push_back(p);
        if(profile[PARENT]){
            bool f = 0;
            for(const auto& p1 : project().root()[PROFILE]){
                if(resolveFromProperties(profile[PARENT].Scalar()) == p1[NAME].Scalar()) f = 1;
                if(f) break;
            }
            if(!f) KEXCEPTION("parent profile not found: "+resolveFromProperties(profile[PARENT].Scalar())+"\n"+project().dir().path());
        }
        if(profile["os"])
            for(const auto& p1 : project().root()[PROFILE])
                if(profile[NAME].Scalar() == p1[NAME].Scalar()) continue;
                else
                if(p1["os"] && p1["os"].Scalar() == profile["os"].Scalar())
                    KEXCEPTION("Multiple os tags with same value found, only one per operating system supported\n"+project().dir().path());
        Validator::PRE_BUILD(*this, profile);
        if(dpp && !dpf) dpf = resolveFromProperties(project().root()[PARENT].Scalar()) == p;
    }
    if(dpp && !dpf) KEXCEPTION("Parent for default profile does not exist: \n"+project().dir().path());
    for(const auto& n : project().root()[PROFILE])
        if(n[SELF] && std::find(profiles.begin(), profiles.end(), resolveFromProperties(n[SELF].Scalar())) == profiles.end())
            KEXCEPTION("Tag self references unknown profile: "+resolveFromProperties(n[SELF].Scalar())+"\n"+project().dir().path());
}

void maiken::Application::postSetupValidation() throw (maiken::Exception){
    Validator::POST_BUILD(*this, project().root());
    for(const auto& profile : project().root()[PROFILE]) Validator::POST_BUILD(*this, profile);
}
