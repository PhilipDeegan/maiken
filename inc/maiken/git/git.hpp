/**
Copyright (c) 2026, Philip Deegan.
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
#ifndef _MAIKEN_GIT_GIT_HPP_
#define _MAIKEN_GIT_GIT_HPP_

#include "mkn/kul/dbg.hpp"
#include "mkn/kul/scm.hpp"

#include "maiken.hpp"

#ifdef _MKN_WITH_MKN_RAM_
#include "maiken/git/github.hpp"
#endif  // _MKN_WITH_MKN_RAM_

namespace maiken {

class Git {
 private:
  static bool IS_SOLID(std::string const& r) {
    return r.find("://") != std::string::npos || r.find("@") != std::string::npos;
  }

  static std::string repo_name(std::string const& repo) {
    auto trimmed = repo;
    while (!trimmed.empty() && trimmed.back() == '/') trimmed.pop_back();
    if (trimmed.find("/") != std::string::npos) {
      auto const name = trimmed.substr(trimmed.rfind("/") + 1);
      return name.empty() ? trimmed : name;
    }
    return repo;
  }

 public:
  // Resolve default branch for the given remote URL via git ls-remote.
  static bool GET_DEFAULT_BRANCH(std::string const& url, std::string& branch) {
    try {
      branch = mkn::kul::scm::Git().defaultRemoteBranch(url);
      return !branch.empty();
    } catch (mkn::kul::Exception const&) {
      return false;
    }
  }

  // Resolve the SCM branch for a dependency.
  // With _MKN_WITH_MKN_RAM_: tries the GitHub API first; falls back to git on API rejection.
  // Without _MKN_WITH_MKN_RAM_: uses git ls-remote directly.
  static std::string resolveSCMBranch(std::string const& repo, std::string const& cacheDirName) {
    MKN_KUL_DBG_FUNC_ENTER;

    bool const module = cacheDirName == "mod";
    auto const name = repo_name(repo);
    mkn::kul::File const verFile{name, ".mkn/" + cacheDirName + "/ver"};
    if (verFile) return mkn::kul::io::Reader(verFile).readLine();

    KOUT(NON) << "Attempting branch deduction resolution for: " << name;
    std::string version;
    bool resolved = false;

#ifdef _MKN_WITH_MKN_RAM_
    try {
      resolved = Github<>::GET_LATEST(repo, version, module);
    } catch (...) {
      KLOG(ERR) << "Github API rejected - falling back to git for: " << name;
    }
#endif  // _MKN_WITH_MKN_RAM_

    if (!resolved) {
      std::vector<std::string> repos;
      if (IS_SOLID(repo))
        repos.push_back(repo);
      else
        for (auto const& s : Settings::INSTANCE().remotes(module)) repos.push_back(s + repo);

      for (std::string const& s : repos) {
        if (GET_DEFAULT_BRANCH(s, version)) {
          resolved = true;
          break;
        }
      }
    }

    if (resolved) {
      verFile.rm();
      verFile.dir().mk();
      mkn::kul::io::Writer(verFile) << version;
      return version;
    }

    return defaultSCMBranchName();
  }
};

}  // namespace maiken

#endif /* _MAIKEN_GIT_GIT_HPP_ */
