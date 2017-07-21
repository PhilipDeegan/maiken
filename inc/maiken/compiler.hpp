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
#ifndef _MAIKEN_COMPILER_HPP_
#define _MAIKEN_COMPILER_HPP_

#include "kul/os.hpp"
#include "kul/cli.hpp"
#include "kul/map.hpp"
#include "kul/proc.hpp"
#include "kul/except.hpp"
#include "kul/string.hpp"

namespace maiken{ 
namespace compiler{
enum Mode { NONE = 0, STAT, SHAR};
}

class Compiler;
class CompilerProcessCapture : public kul::ProcessCapture{
    private:
        std::exception_ptr ep;
        std::string c, f;
    public:
        CompilerProcessCapture() : ep(){}
        CompilerProcessCapture(kul::AProcess& p) : kul::ProcessCapture(p), ep(){}
        CompilerProcessCapture(const CompilerProcessCapture& cp) : kul::ProcessCapture(cp), ep(cp.ep), c(cp.c), f(cp.f){}

        void exception(const std::exception_ptr& e) { ep = e; }
        const std::exception_ptr& exception() const { return ep; }

        void cmd(const std::string& cm) { this->c = cm; }
        const std::string& cmd() const  { return c; }

        void file(const std::string& f) { this->f = f; }
        const std::string& file() const  { return f; }
};

class Compiler{ 
    protected:
        Compiler(const int& v) : version(v){}
        const int version;
    public:
        virtual ~Compiler(){}       
        virtual bool sourceIsBin()      const = 0;
        virtual CompilerProcessCapture buildExecutable(
            const std::string& linker,
            const std::string& linkerEnd,
            const std::vector<std::string>& objects,
            const std::vector<std::string>& libs,
            const std::vector<std::string>& libPaths,
            const std::string& out, 
            const compiler::Mode& mode,
            bool dryRun = false)           const KTHROW(kul::Exception) = 0;
        virtual CompilerProcessCapture buildLibrary(
            const std::string& linker, 
            const std::string& linkerEnd,
            const std::vector<std::string>& objects,
            const std::vector<std::string>& libs,
            const std::vector<std::string>& libPaths,
            const kul::File& out, 
            const compiler::Mode& mode,
            bool dryRun = false)           const KTHROW(kul::Exception) = 0;
        virtual CompilerProcessCapture compileSource  (
            const std::string& compiler,
            const std::vector<std::string>& args,       
            const std::vector<std::string>& incs, 
            const std::string& in, 
            const std::string& out, 
            const compiler::Mode& mode,
            bool dryRun = false)   const KTHROW(kul::Exception) = 0;
        virtual void preCompileHeader(
            const std::vector<std::string>& incs, 
            const std::vector<std::string>& args, 
            const std::string& in, 
            const std::string& out,
            bool dryRun = false)     const KTHROW(kul::Exception) = 0;
};

}
#endif /* _MAIKEN_COMPILER_HPP_ */
