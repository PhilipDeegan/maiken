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
#include "maiken/compiler/csharp.hpp"

maiken::CompilerProcessCapture
maiken::csharp::WINCompiler::buildExecutable(
        const std::string& linker,
        const std::string& linkerEnd,
        const std::vector<std::string>& objects,
        const std::vector<std::string>& libs,
        const std::vector<std::string>& libPaths,
        const std::string& out,
        const maiken::compiler::Mode& mode,
        bool dryRun) const KTHROW(kul::Exception){

    std::string exe = out + ".exe";
    std::string cmd = linker;
    std::vector<std::string> bits;
    if(linker.find(" ") != std::string::npos){
        bits = kul::String::SPLIT(linker, ' ');
        cmd = bits[0];
    }
    kul::Process p(cmd);
    CompilerProcessCapture pc(p);
    for(unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    p.arg("/NOLOGO").arg("/OUT:" + exe);
    if(libs.size()){
        std::stringstream ss;
        for(const std::string& path : libPaths) ss << path << ",";
        std::string s(ss.str());
        s.pop_back();
        p.arg("/LIB:" + s);
        ss.str(std::string());
        for(const std::string& lib : libs)  ss << lib << ".dll,";
        s = ss.str();
        s.pop_back();
        p.arg("/REFERENCE:" + s);
    }

    for(const std::string& o : objects) p.arg(o);
    if(linkerEnd.find(" ") != std::string::npos)
        for(const std::string& s: kul::String::SPLIT(linkerEnd, ' '))
            p.arg(s);
    else p.arg(linkerEnd);

    try{
        if(!dryRun) p.start();
    }catch(const kul::proc::Exception& e){
        pc.exception(std::current_exception());
    }
    pc.file(exe);
    pc.cmd(p.toString());
    return pc;
}

maiken::CompilerProcessCapture
maiken::csharp::WINCompiler::buildLibrary(
        const std::string& linker,
        const std::string& linkerEnd,
        const std::vector<std::string>& objects,
        const std::vector<std::string>& libs,
        const std::vector<std::string>& libPaths,
        const kul::File& out,
        const maiken::compiler::Mode& mode,
        bool dryRun) const KTHROW(kul::Exception){

    std::string dll = out.real() + ".dll";
    std::string cmd = linker;
    std::vector<std::string> bits;
    if(linker.find(" ") != std::string::npos){
        bits = kul::String::SPLIT(linker, ' ');
        cmd = bits[0];
    }
    kul::Process p(cmd);
    p.arg("/target:library").arg("/OUT:" + dll).arg("/nologo");
    CompilerProcessCapture pc(p);
    for(unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    for(const std::string& o : objects) p.arg(o);
    for(const std::string& s: kul::String::SPLIT(linkerEnd, ' ')) p.arg(s);
    try{
        if(!dryRun) p.start();
    }catch(const kul::proc::Exception& e){
        pc.exception(std::current_exception());
    }
    pc.file(dll);
    pc.cmd(p.toString());
    return pc;
}