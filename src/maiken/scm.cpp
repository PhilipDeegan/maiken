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
#include "kul/type.hpp"
#include "maiken.hpp"

void maiken::Application::scmStatus(const bool& deps) throw (kul::scm::Exception){
    if(deps) for(auto app = this->deps.rbegin(); app != this->deps.rend(); ++app) (*app).scmStatus(0);
    if(!scm) scm = SCMGetter::GET(this->project().dir(), this->scr);
    if(scm){
        KOUT(NON) << kul::os::EOL() << "SCM STATUS CHECK ON: " << project().dir().path();
        const std::string& r(this->project().dir().real());
        if(scm->hasChanges(r)) scm->status(r);
        else KOUT(NON) << "NO CHANGES";
    }
}

void maiken::Application::scmUpdate(const bool& f) throw (kul::scm::Exception){
    if(!scm) scm = SCMGetter::GET(this->project().dir(), this->scr);
    if(scm){
        if(!f) KOUT(NON) << "WARNING: ATTEMPTING SCM UPDATE, USER INTERACTION MAY BE REQUIRED!";
        scmUpdate(f, scm, SCMGetter::REPO(this->project().dir(), this->scr));
    }
}

void maiken::Application::scmUpdate(const bool& f, const kul::SCM* scm, const std::string& s) throw (kul::scm::Exception){
    const std::string& ver(this->project().root()[VERSION].Scalar());
    bool c = true;
    if(!f){
        KOUT(NON) << "CHECKING: " << this->project().dir().path() << " FROM " << s;
        const std::string& lV(scm->localVersion(this->project().dir().real(), ver));
        const std::string& rV(scm->remoteVersion(this->project().dir().real(), s, ver));
        c = lV != rV;
        if(!c) KOUT(NON) << "CURRENT VERSION MATCHES REMOTE VERSION: SKIPPING";
        else c = kul::Bool::FROM(kul::cli::receive("UPDATE FROM " + s + " VERSION: " + rV + " (Yes/No/1/0)"));
    }
    if(f || c){
        KOUT(NON) << "UPDATING: " << this->project().dir().path() << " FROM " << s;
        scm->up(this->project().dir().real(), s, ver);
    }
    else KOUT(NON) << "SKIPPED";
}
