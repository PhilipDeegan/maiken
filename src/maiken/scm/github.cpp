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
#include "maiken/github.hpp"

#ifdef _MKN_WITH_MKN_RAM_

bool maiken::Github::GET_DEFAULT_BRANCH(std::string const &owner, std::string const &repo,
                                        std::string &branch) {
  bool b = 0;
  std::stringstream ss;
  ss << "repos/" << owner << "/" << repo;
  kul::https::Get("api.github.com", ss.str())
      .withHeaders({{"User-Agent", "Mozilla not a virus"}, {"Accept", "application/json"}})
      .withResponse([&b, &branch](const kul::http::Response &r) {
        if (r.status() == 200) {
          kul::yaml::String yaml(r.body());
          KLOG(OTH) << "Github API default branch response: " << r.body();
          if (yaml.root() && yaml.root()["default_branch"]) {
            branch = yaml.root()["default_branch"].Scalar();
            b = 1;
          }
        }
      })
      .send();
  return b;
}

bool maiken::Github::GET_LATEST_RELEASE(std::string const &owner, std::string const &repo,
                                        std::string &branch) {
  bool b = 0;
  std::stringstream ss;
  ss << "repos/" << owner << "/" << repo << "/releases/latest";
  kul::https::Get("api.github.com", ss.str())
      .withHeaders({{"User-Agent", "Mozilla not a virus"}, {"Accept", "application/json"}})
      .withResponse([&b, &branch](const kul::http::Response &r) {
        if (r.status() == 200) {
          kul::yaml::String yaml(r.body());
          if (yaml.root()["tag_name"]) {
            branch = yaml.root()["tag_name"].Scalar();
            b = 1;
          }
        }
      })
      .send();
  return b;
}

bool maiken::Github::GET_LATEST_TAG(std::string const &owner, std::string const &repo,
                                    std::string &branch) {
  bool b = 0;
  std::stringstream ss;
  ss << "repos/" << owner << "/" << repo << "/git/tags";
  kul::https::Get("api.github.com", ss.str())
      .withHeaders({{"User-Agent", "Mozilla not a virus"}, {"Accept", "application/json"}})
      .withResponse([&b, &branch](const kul::http::Response &r) {
        if (r.status() == 200) {
          kul::yaml::String yaml(r.body());
          if (yaml.root().Type() == 3) {
            if (yaml.root()["ref"]) {
              branch = yaml.root()["ref"].Scalar();
              b = 1;
            }
          }
        }
      })
      .send();
  if (b == 1) {
    auto bits(kul::String::SPLIT(branch, "/"));
    branch = bits[bits.size() - 1];
  }
  return b;
}

bool maiken::Github::GET_LATEST(std::string const &repo, std::string &branch) {
#ifndef _MKN_DISABLE_SCM_

  std::vector<std::function<decltype(GET_DEFAULT_BRANCH)>> gets{
      &GET_DEFAULT_BRANCH, &GET_LATEST_RELEASE, &GET_LATEST_TAG};
  std::vector<size_t> orders{0, 1, 2};
  if (_MKN_GIT_WITH_RAM_DEFAULT_CO_ACTION_ == 1) orders = {1, 2, 0};

  std::vector<std::string> repos;
  if (IS_SOLID(repo))
    repos.push_back(repo);
  else
    for (std::string const &s : Settings::INSTANCE().remoteRepos()) repos.push_back(s + repo);
  for (std::string const &s : repos) {
    if (s.find("github.com") != std::string::npos) {
      std::string owner = s.substr(s.find("github.com") + 10);
      if (owner[0] != '/' && owner[0] != ':') {
        KERR << "Repo \"" << s << "\" is invalid - skipping";
        continue;
      }
      owner.erase(0, 1);
      if (owner.find("/") != std::string::npos) owner = owner.substr(0, owner.find("/"));

      if (owner.empty()) {
        KERR << "Invalid attempt to perform github lookup";
        continue;
      }

      for (auto const &order : orders)
        if (gets[order](owner, repo, branch)) return 1;
    }
  }
#else
  KEXIT(1,
        "SCM disabled, cannot resolve dependency, check local paths and "
        "configurations");
#endif
  return 0;
}

#endif /*_MKN_WITH_MKN_RAM_*/
