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
#include "kul/log.hpp"
#include "kul/yaml.hpp"

void kul::yaml::File::validate(const YAML::Node& n, const std::vector<NodeValidator>& nvs) throw(Exception) {
    kul::hash::set::String keys;
    for(const auto& nv : nvs) if(nv.name() == "*") return;

    for(YAML::const_iterator it = n.begin(); it != n.end(); ++it){
        const std::string& key(it->first.as<std::string>());
        if(keys.count(key)) KEXCEPTION("Duplicate key detected: " + key + "\n" + this->f);
        keys.insert(key);
        bool f = 0;
        for(const auto& nv : nvs){
            if(nv.name() != key) continue;
            f = 1;
            if(nv.type() == 1 && it->second.Type() != 2) KEXCEPTION("String expected: " + nv.name() + "\n" + this->f);
            if(nv.type() == 2 && it->second.Type() != 3) KEXCEPTION("List expected: " + nv.name() + "\n" + this->f);
            if(nv.type() == 3 && it->second.Type() != 4) KEXCEPTION("Map expected: " + nv.name() + "\n" + this->f);
            if(nv.type() == 2)
                for(size_t i = 0; i < it->second.size(); i++)
                    validate(it->second[i], nv.children());
            if(nv.type() == 3) validate(it->second, nv.children());
        }
        if(!f) KEXCEPTION("Unexpected key: " + key + "\n" + this->f);
    }
    for(const auto& nv : nvs){
        if(nv.mandatory() && !keys.count(nv.name()))
            KEXCEPTION("Key mandatory: : " + nv.name() + "\n" + this->f);
    }
}
