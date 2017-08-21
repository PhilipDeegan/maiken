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
#include <regex>

class Regexer{

    friend class maiken::Application;
    static std::vector<std::string>
    RESOLVE_REGEX(std::string str) KTHROW(kul::Exception){
        std::vector<std::string> v;
        auto posL = str.find("(");
        auto posR = str.find(")");
        if(posL == std::string::npos || posR == std::string::npos) return v;

        if(str.size() > 1 && str.substr(0, 2) == "./") str = str.substr(2);

        kul::Dir d(str);
        std::string built, prnt;
        std::vector<std::string> bits;
        bits.insert(bits.begin(), d.name());
        while(d.parent().name() != prnt && !d.parent().root()){
            bits.insert(bits.begin(), d.parent().name());
            prnt = d.parent().name();
            d = d.parent();
        }
        if(d.parent().root()) {
            bits.insert(bits.begin(), d.parent().name());
        }else{
            d = kul::env::CWD();
            bits.insert(bits.begin(), d.name());
            while(d.parent().name() != prnt && !d.parent().root()){
                bits.insert(bits.begin(), d.parent().name());
                prnt = d.parent().name();
                d = d.parent();
            }
            str = kul::env::CWD() + kul::Dir::SEP() + str;
        }

        std::string rem, rule;
        size_t bitsIndex = 0;
        for(const auto& s : bits){
            auto posL = s.find("(");
            auto posR = s.find(")");
            if(posL != std::string::npos && posR != std::string::npos){
                if(built.size() + s.size() + 2 > str.size()){
                    rem = str.substr(built.size() + s.size() + 1);
                }else
                    rem = str.substr(built.size() + s.size() + 2);
                rule = s;
                break;
            }
            if(kul::Dir(s).root())     built = s;
            else
            if(kul::Dir(built).root()) built = built + s;
            else                       built = built + kul::Dir::SEP() + s;
            bitsIndex++;
        }
        d = built;

        auto regexer = [&](auto items){
            for(const auto& item : items){
                try {
                    std::regex re(rule);
                    std::smatch match;
                    std::string subject(item.name());
                    if(std::regex_search(subject, match, re) && match.size() > 1)
                        RESOLVE_REGEX_REC(item.real(), built, subject, rem, bits, bitsIndex, v);
                } catch (std::regex_error& e) {
                    KEXIT(1, "Regex Failure:\n") << e.what();
                }
            }
        };
        regexer(d.dirs());
        regexer(d.files(0));
        return v;
    }

    static
    void
    RESOLVE_REGEX_REC(
            const std::string& i,
            const std::string& b,
            const std::string& s,
            const std::string& r,
            const std::vector<std::string>& bits,
            const size_t& bitsIndex,
            std::vector<std::string>& v) KTHROW(kul::Exception){

        if(kul::File(i).is() && !kul::Dir(i).is()){
            v.push_back(i);
            return;
        }

        if(bits.size() >= bitsIndex + 1){
            std::string n(kul::Dir::JOIN(b, kul::Dir::JOIN(s, r)));

            const auto again = RESOLVE_REGEX(n);
            if(again.empty()){
                v.push_back(n);
            }else
                for(const auto& v1 : again) v.push_back(v1);
        }
    }

};

void
maiken::Application::addSourceLine(const std::string& s) KTHROW(kul::Exception){
    std::string o = s;
    kul::String::TRIM(o);
    if(o.find(',') == std::string::npos){
        for(const auto& s : kul::cli::asArgs(o)){
            std::string pResolved(Properties::RESOLVE(*this, s));
            kul::Dir d(pResolved);
            if(d.is()) srcs.push_back(std::make_pair(d.real(), true));
            else{
                kul::File f(d.locl());
                if(f) srcs.push_back(std::make_pair(f.real(), false));
                else {
                    auto regexResolved(Regexer::RESOLVE_REGEX(pResolved));
                    if(regexResolved.empty())
                        KEXIT(1, "source does not exist\n"+s+"\n"+project().dir().path());
                    for(const auto & item : regexResolved)
                        srcs.push_back(std::make_pair(item, false));
                }
            }
        }
    }else{
        std::vector<std::string> v;
        kul::String::SPLIT(o, ",", v);
        if(v.size() == 0 || v.size() > 2) KEXIT(1, "source invalid format\n" + project().dir().path());
        kul::Dir d(Properties::RESOLVE(*this, v[0]));
        if(d) srcs.push_back(std::make_pair(d.real(), kul::String::BOOL(v[1])));
        else KEXIT(1, "source does not exist\n"+v[0]+"\n"+project().dir().path());
    }
}

kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String> >
maiken::Application::sourceMap() const{
    const kul::hash::set::String iMs = inactiveMains();
    kul::hash::map::S2T<kul::hash::map::S2T<kul::hash::set::String>> sm;
    if(main.size()){
        kul::File f(kul::Dir(main).locl());
        if(!f)      f = kul::File(main, project().dir());
        if(f.is()) sm[f.name().substr(f.name().rfind(".") + 1)][f.dir().real()].insert(f.real());
        else
        if(!AppVars::INSTANCE().dryRun() && !f.is()) KOUT(NON)  << "WARN : main does not exist: " << f;
    }
    for(const std::pair<std::string, bool>& sourceDir : sources()){
        std::vector<kul::File> files;
        kul::Dir d(sourceDir.first);
        if(d) for(const auto& f : d.files(sourceDir.second)) files.push_back(f);
        else  files.push_back(sourceDir.first);
        for(const kul::File& file : files){
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

bool
maiken::Application::incSrc(const kul::File& file) const {
    bool c = 1;
    if(_MKN_TIMESTAMPS_){
        const std::string& rl(file.mini());
        c = !stss.count(rl);
        if(!c){
            const uint& mod = file.timeStamps().modified();
            if(mod == (*stss.find(rl)).second){
                for(const auto& i : includes()){
                    kul::Dir inc(i.first);
                    if(itss.count(inc.mini()) && includeStamps.count(inc.mini())){
                        if((*includeStamps.find(inc.mini())).second != (*itss.find(inc.mini())).second) c = 1;
                    }else c = 1;
                    if(c) break;
                }
            }else c = 1;
        }
    }
    return c;
}