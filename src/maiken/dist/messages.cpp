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

#include "maiken/dist.hpp"

void maiken::dist::SetupRequest::do_response_for(const mkn::kul::http::A1_1Request& req,
                                                 Sessions& sessions,
                                                 mkn::kul::http::_1_1Response& resp) {
  if (!req.header("session")) KEXCEPTION("BAD 41");  // resp.withBody("FAILUIRE");
  auto session_id = (*req.headers().find("session")).second;

  YAML::Node root;
  bool success = 1;
  if (success)
    root["status"] = 0;
  else {
    root["status"] = 1;
    root["message"] = "NODE BUSY";
  }
  YAML::Emitter out;
  out << root;
  sessions[session_id].reset_setup(this);
  sessions[session_id].setup_ptr()->m_args.erase(STR_NODES);
  sessions[session_id].set_apps(
      maiken::Application::CREATE(sessions[session_id].setup_ptr()->m_args));

  resp.withBody(std::string(out.c_str()));
}

void maiken::dist::CompileRequest::do_response_for(const mkn::kul::http::A1_1Request& req,
                                                   Sessions& sessions,
                                                   mkn::kul::http::_1_1Response& resp) {
  if (!req.header("session")) KEXCEPTION("BAD CompileRequest");
  auto session_id = (*req.headers().find("session")).second;

  mkn::kul::env::CWD(this->m_directory);

  std::vector<mkn::kul::File> cacheFiles;
  sessions[session_id].apps_vector()[0]->compile(this->m_src_obj, sessions[session_id].objects,
                                                 cacheFiles);

  YAML::Node root;
  bool success = 1;
  if (success)
    root["status"] = 0;
  else {
    root["status"] = 1;
    root["message"] = "NODE BUSY";
  }
  root["files"] = this->m_src_obj.size();
  YAML::Emitter out;
  out << root;
  sessions[session_id].m_src_obj = std::move(this->m_src_obj);
  resp.withBody(std::string(out.c_str()));
}

void maiken::dist::LinkRequest::do_response_for(const mkn::kul::http::A1_1Request& req,
                                                Sessions& sessions, mkn::kul::http::_1_1Response& resp) {
  if (!req.header("session")) KEXCEPTION("BAD LinkRequest");
  auto session_id = (*req.headers().find("session")).second;

  Blob b;
  std::istringstream iss(this->str);
  {
    cereal::PortableBinaryInputArchive iarchive(iss);
    iarchive(b);
  }
  mkn::kul::File obj(b.file);
  if (obj) obj = mkn::kul::File(std::string(b.file + ".new"));
  if (!sessions[session_id].binary_writer) {
    obj.rm();
    sessions[session_id].binary_writer = std::make_unique<mkn::kul::io::BinaryWriter>(obj);
  }

  mkn::kul::io::BinaryWriter& bw(*sessions[session_id].binary_writer.get());
  bw.write(b.c1, b.len);
  if (b.last_packet) sessions[session_id].binary_writer.reset();

  YAML::Node root;
  bool success = 1;
  if (success)
    root["status"] = 0;
  else {
    root["status"] = 1;
    root["message"] = "NODE BUSY";
  }
  YAML::Emitter out;
  out << root;
  resp.withBody(std::string(out.c_str()));
}

void maiken::dist::DownloadRequest::do_response_for(const mkn::kul::http::A1_1Request& req,
                                                    Sessions& sessions,
                                                    mkn::kul::http::_1_1Response& resp) {
  if (!req.header("session")) KEXCEPTION("BAD DownloadRequest");
  auto session_id = (*req.headers().find("session")).second;

  auto& src_obj = sessions[session_id].m_src_obj;
  if (!sessions[session_id].binary_reader) {
    if (!src_obj.empty()) {
      auto& pair = src_obj[0];
      mkn::kul::File bin(pair.second);
      sessions[session_id].binary_reader = std::make_unique<mkn::kul::io::BinaryReader>(bin);
    }
  }

  Blob b;
  b.files_left = src_obj.size();
  bzero(b.c1, BUFF_SIZE);
  b.file = src_obj[0].second;
  auto& br(*sessions[session_id].binary_reader.get());
  size_t red = b.len = br.read(b.c1, BUFF_SIZE);
  if (red == 0) {
    src_obj.erase(src_obj.begin());
    b.files_left = src_obj.size();
    sessions[session_id].binary_reader.reset();
    // if (b.files_left == 0) sessions.erase(req.ip());
    b.last_packet = 1;
  }
  std::ostringstream ss(std::ios::out | std::ios::binary);
  {
    cereal::PortableBinaryOutputArchive oarchive(ss);
    oarchive(b);
  }
  resp.withBody(ss.str());
}

#endif  // _MKN_WITH_MKN_RAM_ && _MKN_WITH_IO_CEREAL_
