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
#include "maiken/except.hpp"
#include "maiken/scm.hpp"
#include "maiken/settings.hpp"

const kul::SCM* maiken::SCMGetter::GET_SCM(const kul::Dir& d,
                                           const std::string& r, bool module) {
  std::vector<std::string> repos;
  if (IS_SOLID(r))
    repos.push_back(r);
  else if (module)
    for (const std::string& s : Settings::INSTANCE().remoteModules())
      repos.push_back(s + r);
  else
    for (const std::string& s : Settings::INSTANCE().remoteRepos())
      repos.push_back(s + r);
#ifndef _MKN_DISABLE_SCM_
  for (const auto& repo : repos) {
#ifndef _MKN_DISABLE_GIT_
    try {
      kul::Process g("git");
      kul::ProcessCapture gp(g);
      std::string r1(repo);
      if (repo.find("http") != std::string::npos &&
          repo.find("@") == std::string::npos)
        r1 = repo.substr(0, repo.find("//") + 2) + "u:p@" +
             repo.substr(repo.find("//") + 2);
      g.arg("ls-remote").arg(r1);
      g.start();
      auto errs(gp.errs());
      kul::String::TRIM(errs);
      auto lines(kul::String::LINES(errs));
      if (errs.empty()) lines.clear();
      bool allwarn = true;
      for (const auto& line : lines)
        if (line.find("warning") == std::string::npos) allwarn = false;
      if (lines.empty() || (lines.size() && allwarn)) {
        INSTANCE().valids.insert(d.path(), repo);
        return &kul::scm::Manager::INSTANCE().get("git");
      }
      KLOG(DBG) << gp.outs();
      KLOG(DBG) << gp.errs();
    } catch (const kul::proc::ExitException& e) {
      KLOG(ERR) << e.stack();
    }
#endif  //_MKN_DISABLE_GIT_
        // SVN NOT YET SUPPORTED
        // #ifndef _MKN_DISABLE_SVN_
        //                 try{
        //                    kul::Process s("svn");
        //                    kul::ProcessCapture sp(s);
        //                    s.arg("ls").arg(repo).start();
        //                    if(!sp.errs().size()) {
        //                        INSTANCE().valids.insert(d.path(), repo);
    //                        return &kul::scm::Manager::INSTANCE().get("svn");
    //                    }
    //                 }catch(const kul::proc::ExitException& e){}
    // #endif//_MKN_DISABLE_SVN_
  }
#else
  KEXIT(1,
        "SCM disabled, cannot resolve dependency, check local paths and "
        "configurations");
#endif  //_MKN_DISABLE_SCM_
  std::stringstream ss;
  for (const auto& s : repos) ss << s << "\n";
  KEXIT(1, "SCM not found or not supported type(git/svn) for repo(s)\n\t" +
               ss.str() + "\tproject: " + d.path());
}

bool maiken::SCMGetter::IS_SOLID(const std::string& r) {
  return r.find("://") != std::string::npos || r.find("@") != std::string::npos;
}

std::string maiken::SCMGetter::REPO(const kul::Dir& d, const std::string& r,
                                    bool module) {
  if (INSTANCE().valids.count(d.path()))
    return (*INSTANCE().valids.find(d.path())).second;
  if (IS_SOLID(r))
    INSTANCE().valids.insert(d.path(), r);
  else
    GET_SCM(d, r, module);
  if (INSTANCE().valids.count(d.path()))
    return (*INSTANCE().valids.find(d.path())).second;
  KEXCEPT(Exception, "SCM not discovered for project: " + d.path());
}
bool maiken::SCMGetter::HAS(const kul::Dir& d) {
  return (kul::Dir(d.join(".git")) || kul::Dir(d.join(".svn")));
}
const kul::SCM* maiken::SCMGetter::GET(const kul::Dir& d, const std::string& r,
                                       bool module) {
  if (IS_SOLID(r)) INSTANCE().valids.insert(d.path(), r);
  if (kul::Dir(d.join(".git")))
    return &kul::scm::Manager::INSTANCE().get("git");
  if (kul::Dir(d.join(".svn")))
    return &kul::scm::Manager::INSTANCE().get("svn");
  return r.size() ? GET_SCM(d, r, module) : 0;
}
