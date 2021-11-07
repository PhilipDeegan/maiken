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
#ifndef _MAIKEN_DIST_MESSAGE_HPP_
#define _MAIKEN_DIST_MESSAGE_HPP_

namespace maiken {
namespace dist {

class Post;
class Server;
class ServerSession;

using Sessions = std::unordered_map<std::string, ServerSession>;

class AMessage : public Constants {
  friend class ::cereal::access;

 public:
  virtual ~AMessage() {}
  std::string clazz_name() const { return std::string(typeid(*this).name()); }

 protected:
  void build_from(YAML::Node const&& node) {
    YAML::Emitter out;
    out << node;
    str = out.c_str();
  }

 protected:
  std::string str;

 private:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(str);
  }
};

class ARequest : public AMessage {
  friend class ::cereal::access;

 public:
  ARequest() {}
  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("AMessage", ::cereal::base_class<AMessage>(this)));
  }

  virtual void do_response_for(const mkn::kul::http::A1_1Request& req, Sessions& sessions,
                               mkn::kul::http::_1_1Response& resp) = 0;
};

class SetupRequest : public ARequest {
  friend class ::cereal::access;
  friend class RemoteCommandManager;
  friend class Server;

 public:
  SetupRequest() {}
  SetupRequest(std::string const& project, std::string const& settings, mkn::kul::cli::Args const& args)
      : m_project_yaml(project), m_settings_yaml(settings), m_args(args) {}

  void do_response_for(const mkn::kul::http::A1_1Request& req, Sessions& sessions,
                       mkn::kul::http::_1_1Response& resp) override;

 private:
  SetupRequest(const SetupRequest&) = delete;
  SetupRequest(const SetupRequest&&) = delete;
  SetupRequest& operator=(const SetupRequest&) = delete;
  SetupRequest& operator=(const SetupRequest&&) = delete;
  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("ARequest", ::cereal::base_class<ARequest>(this)));
    ar(::cereal::make_nvp("m_project_yaml", m_project_yaml));
    ar(::cereal::make_nvp("m_settings_yaml", m_settings_yaml));
    ar(::cereal::make_nvp("m_args", m_args));
  }

 private:
  std::string m_project_yaml, m_settings_yaml;
  mkn::kul::cli::Args m_args;
};

class CompileRequest : public ARequest {
  friend class ::cereal::access;
  friend class RemoteCommandManager;
  friend class Server;

 public:
  CompileRequest() {}
  CompileRequest(std::string const& directory,
                 const std::vector<std::pair<std::string, std::string>>& src_obj)
      : m_directory(directory), m_src_obj(src_obj) {}

  void do_response_for(const mkn::kul::http::A1_1Request& req, Sessions& sessions,
                       mkn::kul::http::_1_1Response& resp) override;

 private:
  CompileRequest(const CompileRequest&) = delete;
  CompileRequest(const CompileRequest&&) = delete;
  CompileRequest& operator=(const CompileRequest&) = delete;
  CompileRequest& operator=(const CompileRequest&&) = delete;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("ARequest", ::cereal::base_class<ARequest>(this)));
    ar(::cereal::make_nvp("m_src_obj", m_src_obj));
    ar(::cereal::make_nvp("m_directory", m_directory));
  }

 private:
  std::string m_directory;
  std::vector<std::pair<std::string, std::string>> m_src_obj;
};

class DownloadRequest : public ARequest {
  friend class ::cereal::access;
  friend class RemoteCommandManager;
  friend class Server;

 public:
  DownloadRequest() {}
  void do_response_for(const mkn::kul::http::A1_1Request& req, Sessions& sessions,
                       mkn::kul::http::_1_1Response& resp) override;

 private:
  DownloadRequest(const DownloadRequest&) = delete;
  DownloadRequest(const DownloadRequest&&) = delete;
  DownloadRequest& operator=(const DownloadRequest&) = delete;
  DownloadRequest& operator=(const DownloadRequest&&) = delete;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("ARequest", ::cereal::base_class<ARequest>(this)));
  }
};

class LinkRequest : public ARequest {
  friend class ::cereal::access;
  friend class RemoteCommandManager;
  friend class Server;

 public:
  LinkRequest() {}
  LinkRequest(std::string const& b) { str = b; }
  void do_response_for(const mkn::kul::http::A1_1Request& req, Sessions& sessions,
                       mkn::kul::http::_1_1Response& resp) override;

 private:
  LinkRequest(const LinkRequest&) = delete;
  LinkRequest(const LinkRequest&&) = delete;
  LinkRequest& operator=(const LinkRequest&) = delete;
  LinkRequest& operator=(const LinkRequest&&) = delete;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("ARequest", ::cereal::base_class<ARequest>(this)));
  }
};

class Blob {
 public:
  bool last_packet = 0;
  size_t len = 0, files_left = 1;
  uint8_t* c1 = new uint8_t[BUFF_SIZE];
  std::string file;
  Blob() {}
  ~Blob() { delete[] c1; }
  template <class Archive>
  void serialize(Archive& ar) {
    ar(last_packet);
    ar(cereal::make_size_tag(files_left));
    ar(cereal::make_size_tag(len));
    ar(cereal::binary_data(c1, sizeof(uint8_t) * len));
    ar(file);
  }

 private:
  Blob(const Blob&) = delete;
  Blob(const Blob&&) = delete;
  Blob& operator=(const Blob&) = delete;
  Blob& operator=(const Blob&&) = delete;
};
}  // end namespace dist
}  // end namespace maiken

#endif  // _MAIKEN_DIST_MESSAGE_HPP_
