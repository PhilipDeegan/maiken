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
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

#include "maiken.hpp"
#include "maiken/dist.hpp"

std::unique_ptr<maiken::dist::SetupRequest> maiken::dist::RemoteCommandManager::build_setup_query(
    maiken::Application const& a, mkn::kul::cli::Args const& args) {
  mkn::kul::os::PushDir pushd(a.project().dir());
  YAML::Node root;
  root["project"] = a.project().root()["name"].Scalar();
  root["directory"] = a.project().dir().real();

  if (a.project().root()["scm"]) {
    root["scm"] = a.project().root()["scm"].Scalar();
  } else if (mkn::kul::Dir(".git")) {
    root["scm"] = mkn::kul::scm::Git().origin(mkn::kul::env::CWD());
  }

  std::string project_string, settings_string;
  {
    YAML::Emitter project;
    project << root;
    project_string = std::string(project.c_str());
  }
  {
    YAML::Emitter settings;
    settings << maiken::Settings::INSTANCE().root();
    settings_string = std::string(settings.c_str());
  }
  root["settings_file"] = maiken::Settings::INSTANCE().file();

  return std::make_unique<maiken::dist::SetupRequest>(project_string, settings_string, args);
}

std::unique_ptr<maiken::dist::CompileRequest>
maiken::dist::RemoteCommandManager::build_compile_request(
    std::string const& directory,
    const std::vector<std::pair<std::string, std::string>>& src_objs) {
  return std::make_unique<maiken::dist::CompileRequest>(directory, src_objs);
}

std::unique_ptr<maiken::dist::DownloadRequest>
maiken::dist::RemoteCommandManager::build_download_request() {
  return std::make_unique<maiken::dist::DownloadRequest>();
}

std::unique_ptr<maiken::dist::LinkRequest> maiken::dist::RemoteCommandManager::build_link_request(
    std::string const& b) {
  return std::make_unique<maiken::dist::LinkRequest>(b);
}

#endif  // _MKN_WITH_MKN_RAM_ && _MKN_WITH_IO_CEREAL_
