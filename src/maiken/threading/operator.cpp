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

void maiken::ThreadingCompiler::operator()() KTHROW(kul::Exception){
    std::pair<std::string, std::string> p;
    {
        kul::ScopeLock lock(compile);
        p = sources.front();
        sources.pop();
    }
    const std::string src(p.first);
    const std::string obj(p.second);
    if(!f){
        const std::string& fileType = src.substr(src.rfind(".") + 1);
        const std::string& compiler = (*(*app.files().find(fileType)).second.find(STR_COMPILER)).second;
        std::vector<std::string> args;
        if(app.arguments().count(fileType) > 0)
            for(const std::string& o : (*app.arguments().find(fileType)).second)
                for(const auto& s : kul::cli::asArgs(o))
                    args.push_back(s);
        for(const auto& s : kul::cli::asArgs(app.arg)) args.push_back(s);
        std::string cmd = compiler + " " + AppVars::INSTANCE().args();
        if(AppVars::INSTANCE().jargs().count(fileType) > 0)
            cmd += " " + (*AppVars::INSTANCE().jargs().find(fileType)).second;
        // WE CHECK BEFORE USING THIS THAT A COMPILER EXISTS FOR EVERY FILE
        const kul::code::CompilerProcessCapture& cpc
            = kul::code::Compilers::INSTANCE().get(compiler)
                ->compileSource(cmd, args, incs, src, obj, app.m, AppVars::INSTANCE().dryRun());
        kul::ScopeLock lock(push);
        cpcs.push_back(cpc);
        if(cpc.exception()) f = 1;
    }
}