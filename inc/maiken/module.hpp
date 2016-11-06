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
#ifndef _MAIKEN_PLUGIN_HPP_
#define _MAIKEN_PLUGIN_HPP_

#include "kul/os.hpp"
#include "kul/log.hpp"
#include "kul/sys.hpp"
#include "kul/yaml.hpp"

namespace maiken{ 
class Plugin;
}

/*
extern "C" 
KUL_PUBLISH 
void maiken_module_construct(maiken::Plugin* p);

extern "C" 
KUL_PUBLISH  
void maiken_module_destruct(maiken::Plugin* p);
*/

namespace maiken{ 

class Application; 

class ModuleException : public kul::Exception{
    public:
        ModuleException(const char*f, const uint16_t& l, const std::string& s) : kul::Exception(f, l, s){}
};

enum MODULE_PHASE {
    COMPILE = 0, LINK, PACK
};


class KUL_PUBLISH Module {

    public:
        virtual ~Module(){}
        virtual void execute(Application& app) = 0;
        // virtual void compile(const Application& app, const kuL::File& f) throw(ModuleException){}
        // virtual void link   (const Application& app, const kuL::File& f) throw(ModuleException){}
        // virtual void pack   (const Application& app, const kuL::File& f) throw(ModuleException){}

};

class KUL_PUBLISH ModuleLoader : public kul::sys::SharedClass<maiken::Module> {
    private:
        bool loaded = 0;
        Module* p;
    public:
        ModuleLoader(const kul::File& f) throw(kul::sys::Exception) 
            : kul::sys::SharedClass<maiken::Module>(f, "maiken_module_construct", "maiken_module_destruct") {
            construct(p);
            loaded = 1;
        }
        ~ModuleLoader(){
            if(loaded) KERR << "WARNING: ModuleLoader not unloaded, possible memory leak";
        }
        void unload(){
            destruct(p);
            loaded = 0;
        }
        void execute(Application& app){
            if(!p) KEXCEPTSTR(ModuleException) << "PLUGIN NOT LOADED";
            p->execute(app);
        }

};


}
#endif /* _MAIKEN_PROJECT_HPP_ */
