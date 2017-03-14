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

void maiken::Application::loadTimeStamps() throw (kul::StringException){
    if(_MKN_TIMESTAMPS_){
        kul::Dir mkn(buildDir().join(".mkn"));
        kul::File src("src_stamp", mkn);
        if(mkn && src){
            kul::io::Reader r(src);
            const char* c = 0;
            while((c = r.readLine())){
                std::string s(c);
                if(s.size() == 0) continue;
                std::vector<std::string> bits;
                kul::String::SPLIT(s, ' ', bits);
                if(bits.size() != 2) KEXCEPTION("timestamp file invalid format\n"+src.full());
                try{
                    stss.insert(bits[0], kul::String::UINT16(bits[1]));
                }catch(const kul::StringException& e){
                    KEXCEPTION("timestamp file invalid format\n"+src.full());
                }
            }
        }
        kul::File inc("inc_stamp", mkn);
        if(mkn && inc){
            kul::io::Reader r(inc);
            const char* c = 0;
            while((c = r.readLine())){
                std::string s(c);
                if(s.size() == 0) continue;
                std::vector<std::string> bits;
                kul::String::SPLIT(s, ' ', bits);
                if(bits.size() != 2) KEXCEPTION("timestamp file invalid format\n"+inc.full());
                try{
                    itss.insert(bits[0], bits[1]);
                }catch(const kul::StringException& e){
                    KEXCEPTION("timestamp file invalid format\n"+src.full());
                }
            }
        }
        for(const auto& i : includes()){
            kul::Dir inc(i.first);
            ulonglong includeStamp = inc.timeStamps().modified();
            for(const auto fi : inc.files(1)) includeStamp += fi.timeStamps().modified();
            std::ostringstream os;
            os << std::hex << includeStamp;
            includeStamps.insert(inc.mini(), os.str());
        }
    }
}