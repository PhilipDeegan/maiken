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
#ifndef _MAIKEN_DIST_HPP_
#define _MAIKEN_DIST_HPP_
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include <cereal/archives/portable_binary.hpp>

#include "kul/cli.hpp"
#include "kul/http.hpp"
#include "kul/io.hpp"
#include "kul/threads.hpp"
#include "kul/time.hpp"
#include "kul/yaml.hpp"

#include "maiken.hpp"

namespace maiken {
namespace dist {
class Exception : public kul::Exception {
 public:
  Exception(const char *f, const uint16_t &l, const std::string &s)
      : kul::Exception(f, l, s) {}
};
class RemoteCommandManager;
class Server;
}  // namespace dist
}  // namespace maiken

#include "maiken/dist/message.hpp"
#include "maiken/dist/server.hpp"

namespace maiken {
class Application;
namespace dist {

class Host {
 private:
  uint16_t m_port;
  std::string m_host;

 public:
  Host(std::string host, uint16_t port) : m_port(port), m_host(host) {}
  const std::string &host() const { return m_host; }
  const uint16_t &port() const { return m_port; }
  const std::string session_id() const {
    std::stringstream ss;
    ss << std::hex << m_host << this;
    return ss.str();
  }
};

class Post {
  friend class ::cereal::access;
  friend class Server;

 public:
  ~Post() { KLOG(INF); }
  explicit Post(ARequest *_msg) : msg(std::unique_ptr<ARequest>(_msg)) {
    if (msg == nullptr) KLOG(INF) << "NOOOOOOOOOOO";
    KLOG(INF);
  }

  explicit Post(std::unique_ptr<ARequest> _msg) : msg(std::move(_msg)) {
    if (msg == nullptr) KLOG(INF) << "NOOOOOOOOOOO";
    KLOG(INF);
  }
  void send(const Host &host) KTHROW(Exception) {
    send(host.host(), "res", host.port(), {{"session", host.session_id()}});
  }
  void send(const std::string &host, const std::string &res,
            const uint16_t &port,
            const std::unordered_map<std::string, std::string> headers = {{}}) KTHROW(maiken::Exception);
  ARequest *message() { return msg.get(); }

  const std::string &body() { return _body; }

  void release() { msg.release(); }

 private:
  Post(){}
  Post(const Post &) = delete;
  Post(const Post &&) = delete;
  Post &operator=(const Post &) = delete;
  Post &operator=(const Post &&) = delete;
  template <class Archive>
  void serialize(Archive &ar) {
    ar(::cereal::make_nvp("msg", msg));
  }

 private:
  std::string _body;
  std::unique_ptr<ARequest> msg = nullptr;
};

class RemoteCommandManager {
 public:
  static RemoteCommandManager &INST() {
    static RemoteCommandManager inst;
    return inst;
  }

  std::unique_ptr<SetupRequest> build_setup_query(const maiken::Application &a,
                                                  const kul::cli::Args &args);

  std::unique_ptr<CompileRequest> build_compile_request(
      const std::string &directory,
      const std::vector<std::pair<std::string, std::string> > &src_obj);

  std::unique_ptr<DownloadRequest> build_download_request();

  void build_hosts(const Settings &settings) KTHROW(kul::Exception) {
    if (settings.root()["dist"]) {
      if (settings.root()["dist"]["nodes"]) {
        for (const auto &node : settings.root()["dist"]["nodes"]) {
          m_hosts.emplace_back(node["host"].Scalar(),
                               kul::String::UINT16(node["port"].Scalar()));
        }
      }
    }
  }
  const std::vector<Host> &hosts() const { return m_hosts; }

 private:
  std::vector<Host> m_hosts;
};
using RMC = RemoteCommandManager;

}  // end namespace dist
}  // end namespace maiken

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::AMessage,
                                   cereal::specialization::member_serialize)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::ARequest,
                                   cereal::specialization::member_serialize)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::SetupRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::SetupRequest)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::CompileRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::CompileRequest)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::DownloadRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::DownloadRequest)

#endif  // _MKN_WITH_MKN_RAM_ && _MKN_WITH_IO_CEREAL_
#endif  // _MAIKEN_DIST_HPP_
