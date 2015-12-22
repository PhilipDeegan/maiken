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
#ifndef _KUL_YAML_HPP_
#define _KUL_YAML_HPP_

#include "kul/io.hpp"
#include "kul/hash.hpp"
#include "kul/except.hpp"

#include "yaml-cpp/yaml.h"

namespace kul{ namespace yaml{

class NodeValidator;

class Exception : public kul::Exception{
    public:
        Exception(const char*f, const int l, const std::string& s) : kul::Exception(f, l, s){}
};

enum NodeType { NON = 0, STRING, LIST, MAP };

class FileValidator{
    private:
        const std::vector<NodeValidator> kids;
    public:
        FileValidator(const std::vector<NodeValidator>& kids) : kids(kids){}
        const std::vector<NodeValidator>&   children()  const { return this->kids; }
};

class File{
    private:        
        const std::string f;
        YAML::Node r;
        void validate(const YAML::Node& n, const std::vector<NodeValidator>& nvs) throw(Exception);
    protected:
        void reload() throw(Exception){
            try{
                r = YAML::LoadFile(f); 
            }catch(const std::exception& e){ KEXCEPTION("YAML failed to parse\nFile:"+f); }
        }
        File(const File& f) : f(f.f), r(f.r){}
        File(const std::string& f) throw(Exception) : f(f) {
            reload();
        }   
    public:
        template <class T> static T CREATE(const std::string& f) throw(Exception) {
            T file(f);          
            file.validate(file.root(), file.validator().children());
            return file;
        }
        virtual ~File(){}
        const virtual FileValidator validator() = 0;
        const YAML::Node&           root() const { return r; }
        const std::string&          file() const { return f; }
};

class NodeValidator{
    private:        
        bool m;
        NodeType typ;
        std::string nam;
        std::vector<NodeValidator> kids;
    public:
        NodeValidator(const std::string n, bool m = 0) : m(m), typ(STRING), nam(n){}
        NodeValidator(const std::string n, 
                const std::vector<NodeValidator>& c,
                bool m, const NodeType& typ) :
                    m(m), typ(typ), nam(n), kids(c){}
        const std::vector<NodeValidator>&   children()  const { return this->kids; }
        bool                                mandatory() const { return this->m; }
        const std::string&                  name()      const { return this->nam; }
        const NodeType&                     type()      const { return this->typ; }
};


}}
#endif /* _KUL_YAML_HPP_ */
