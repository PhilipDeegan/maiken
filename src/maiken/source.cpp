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

void maiken::Application::addSourceLine(const std::string& s) KTHROW(kul::Exception){
    std::string o = s;
    kul::String::TRIM(o);
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
