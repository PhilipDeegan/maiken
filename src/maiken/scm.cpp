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

class UpdateTracker {
 private:
  kul::hash::set::String paths;

 public:
  bool has(const std::string& path) { return paths.count(path); }
  void add(const std::string& path) { paths.insert(path); }
  static UpdateTracker& INSTANCE() {
    static UpdateTracker i;
    return i;
  }
};

void maiken::Application::scmStatus(const bool& deps)
    KTHROW(kul::scm::Exception) {
  std::vector<Application*> v;
  if (deps)
    for (auto app = this->deps.rbegin(); app != this->deps.rend(); ++app) {
      const std::string& s((*app)->project().dir().real());
      auto it = std::find_if(v.begin(), v.end(), [&s](const Application* a) {
        return a->project().dir().real() == s;
      });
      if (it == v.end() &&
          (*app)->project().dir().real() != this->project().dir().real())
        v.push_back(*app);
    }

  for (auto* app : v) app->scmStatus(0);
  if (!scm && SCMGetter::HAS(this->project().dir()))
    scm = SCMGetter::GET(this->project().dir(), this->scr, isMod);
  if (scm) {
    KOUT(NON) << "SCM STATUS CHECK ON: " << project().dir().real();
    const std::string& r(this->project().dir().real());
    scm->status(r);
  }
}

void maiken::Application::scmUpdate(const bool& f) KTHROW(kul::scm::Exception) {
  uint i = 0;
  const Application* p = this;
  while ((p = p->par)) i++;
  if (i > AppVars::INSTANCE().dependencyLevel()) return;
  if (!scm && SCMGetter::HAS(this->project().dir()))
    scm = SCMGetter::GET(this->project().dir(), this->scr, isMod);
  if (scm && !UpdateTracker::INSTANCE().has(this->project().dir().real())) {
    if (!f)
      KOUT(NON) << "WARNING: ATTEMPTING SCM UPDATE, USER INTERACTION MAY BE "
                   "REQUIRED!";

    const std::string& tscr(
        !this->scr.empty()
            ? this->scr
            : this->project().root()[STR_SCM]
                  ? Properties::RESOLVE(
                        *this, this->project().root()[STR_SCM].Scalar())
                  : this->project().root()[STR_NAME].Scalar());

    scmUpdate(f, scm, SCMGetter::REPO(this->project().dir(), tscr, isMod));
    UpdateTracker::INSTANCE().add(this->project().dir().real());
  }
}

void maiken::Application::scmUpdate(const bool& f, const kul::SCM* scm,
                                    const std::string& url)
    KTHROW(kul::scm::Exception) {
  const std::string& ver(
      !this->scv.empty() ? this->scv
                         : this->project().root()[STR_VERSION]
                               ? this->project().root()[STR_VERSION].Scalar()
                               : "");
  bool c = true;
  if (!f) {
    KOUT(NON) << "CHECKING: " << this->project().dir().real() << " FROM "
              << url;
    const std::string& lV(scm->localVersion(this->project().dir().real(), ver));
    const std::string& rV(url.size() ? scm->remoteVersion(url, ver) : "");
    c = lV != rV;
    std::stringstream ss;
    ss << "UPDATE FROM " << url << " VERSION: " << rV << " (Yes/No/1/0)";
    if (!c)
      KOUT(NON) << "CURRENT VERSION MATCHES REMOTE VERSION: SKIPPING";
    else
      c = kul::String::BOOL(kul::cli::receive(ss.str()));
  }
  if (f || c) {
    std::stringstream ss;
    if (url.size()) ss << " FROM " << url;
    KOUT(NON) << "UPDATING: " << this->project().dir().real() << ss.str();
    scm->up(this->project().dir().real(), url, ver);
    kul::os::PushDir pushd(this->project().dir());
    kul::Dir build(".mkn/build");
    kul::File ts("timestamp", build);
    if (build && ts) ts.rm();
  }
}
