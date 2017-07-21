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
#ifndef _MAIKEN_CODE_COMPILERS_HPP_
#define _MAIKEN_CODE_COMPILERS_HPP_

#include "maiken/compiler/cpp.hpp"
#include "maiken/compiler/csharp.hpp"

namespace maiken{ 
namespace compiler{
class Exception : public kul::Exception{
    public:
        Exception(const char*f, const int l, std::string s) : kul::Exception(f, l, s){}
};
}

class CompilerNotFoundException : public kul::Exception{
    public:
        CompilerNotFoundException(const char*f, const int l, std::string s) : kul::Exception(f, l, s){}
};

template<typename T, typename... Args> 
std::unique_ptr<T> make_unique(Args&&... args){
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Compilers{
    private:
        Compilers(){
            clang   = make_unique<cpp::ClangCompiler>();
            gcc     = make_unique<cpp::GccCompiler>();
            hcc     = make_unique<cpp::HccCompiler>();

            intel   = make_unique<cpp::IntelCompiler>();
            winc    = make_unique<cpp::WINCompiler>();

            wincs   = make_unique<csharp::WINCompiler>();

            cs.insert(std::pair<std::string, Compiler*>("cl"        , winc.get()));
            cs.insert(std::pair<std::string, Compiler*>("csc"       , wincs.get()));

            cs.insert(std::pair<std::string, Compiler*>("clang"     , clang.get()));
            cs.insert(std::pair<std::string, Compiler*>("clang++"   , clang.get()));

            cs.insert(std::pair<std::string, Compiler*>("gcc"       , gcc.get()));
            cs.insert(std::pair<std::string, Compiler*>("g++"       , gcc.get()));

            cs.insert(std::pair<std::string, Compiler*>("hcc"       , hcc.get()));
            cs.insert(std::pair<std::string, Compiler*>("h++"       , hcc.get()));

            cs.insert(std::pair<std::string, Compiler*>("icc"       , intel.get()));
            cs.insert(std::pair<std::string, Compiler*>("icpc"      , intel.get()));

            cs.insert(std::pair<std::string, Compiler*>("nvcc"      , gcc.get()));
        }
        std::unique_ptr<Compiler> hcc;
        std::unique_ptr<Compiler> gcc;
        std::unique_ptr<Compiler> clang;
        std::unique_ptr<Compiler> intel;
        std::unique_ptr<Compiler> winc;
        std::unique_ptr<Compiler> wincs;

        kul::hash::map::S2T<Compiler*> cs, masks;

        const std::string key(std::string comp, const kul::hash::map::S2T<Compiler*>& map){
            kul::String::REPLACE_ALL(comp, ".exe", "");
            if(map.count(comp) > 0) return comp;
            if(comp.find(" ") != std::string::npos)
                for(const std::string& s :kul::String::SPLIT(comp, ' ')){
                    if(map.count(s) > 0) return s;
                    if(std::string(kul::Dir(s).locl()).find(kul::Dir::SEP()) != std::string::npos)
                        if(map.count(s.substr(s.rfind(kul::Dir::SEP()) + 1)))
                            return s.substr(s.rfind(kul::Dir::SEP()) + 1);
                }
            if(std::string(kul::Dir(comp).locl()).find(kul::Dir::SEP()) != std::string::npos){
                comp = comp.substr(comp.rfind(kul::Dir::SEP()) + 1);
                if(map.count(comp)) return comp;
            }

            KEXCEPT(CompilerNotFoundException, "Compiler for " + comp + " is not implemented");
        }
    public:
        static Compilers& INSTANCE(){ 
            static Compilers instance;
            return instance;
        }
        const std::vector<std::string> keys(){
            std::vector<std::string> ks;
            for(const auto& p : cs) ks.push_back(p.first);
            return ks;
        }
        void addMask(const std::string& m, const std::string& c) KTHROW(CompilerNotFoundException){
            const std::string k(key(c, cs));
            if(cs.count(m)) KEXCEPT(compiler::Exception, "Mask cannot replace compiler");
            masks[m] = cs[k];
        }
        const Compiler* get(const std::string& comp) KTHROW(CompilerNotFoundException){
            try{
                return cs[key(comp, cs)];
            }catch(const CompilerNotFoundException& e){}
            return masks[key(comp, masks)];
        }
};

}
#endif /* _MAIKEN_CODE_COMPILERS_HPP_ */
