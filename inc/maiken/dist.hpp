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

#include "mkn/kul/cli.hpp"
#include "mkn/kul/http.hpp"
#include "mkn/kul/io.hpp"
#include "mkn/kul/threads.hpp"
#include "mkn/kul/time.hpp"
#include "mkn/kul/yaml.hpp"

#include "maiken.hpp"

namespace maiken {
namespace dist {

const constexpr size_t BUFF_SIZE = (_KUL_TCP_READ_BUFFER_ - 666);

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s) : mkn::kul::Exception(f, l, s) {}
};
class RemoteCommandManager;
class Server;
}  // namespace dist

class DistLinker {
 public:
  static void send([[maybe_unused]] mkn::kul::File const& bin) {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
    std::vector<std::shared_ptr<maiken::dist::Post>> posts;
    auto post_lambda = [](const dist::Host& host, mkn::kul::File const& bin) {
      mkn::kul::io::BinaryReader br(bin);
      dist::Blob b;
      b.files_left = 1;
      b.file = bin.real();
      size_t red = 0;
      do {
        bzero(b.c1, dist::BUFF_SIZE / 2);
        red = b.len = br.read(b.c1, dist::BUFF_SIZE / 2);
        if (red == 0) {
          b.last_packet = 1;
          b.files_left = 0;
        }
        std::ostringstream ss(std::ios::out | std::ios::binary);
        {
          cereal::PortableBinaryOutputArchive oarchive(ss);
          oarchive(b);
        }
        auto link = std::make_shared<maiken::dist::Post>(
            maiken::dist::RemoteCommandManager::INST().build_link_request(ss.str()));
        link->send(host);
      } while (red > 0);
    };

    auto& hosts(maiken::dist::RemoteCommandManager::INST().hosts());
    size_t threads = hosts.size();
    mkn::kul::ChroncurrentThreadPool<> ctp(threads, 1, 1000000, 1000);
    auto post_ex = [&](mkn::kul::Exception const& e) {
      ctp.stop().interrupt();
      throw e;
    };
    for (size_t i = 0; i < threads; i++) {
      ctp.async(std::bind(post_lambda, std::ref(hosts[i]), std::ref(bin)), post_ex);
    }
    ctp.finish(10000000);  // 10 milliseconds
    ctp.rethrow();
#endif  //  _MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  }
};

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
  std::string const& host() const { return m_host; }
  uint16_t const& port() const { return m_port; }
  std::string const session_id() const {
    std::stringstream ss;
    ss << std::hex << m_host << this;
    return ss.str();
  }
};

class Post {
  friend class ::cereal::access;
  friend class Server;

 public:
  ~Post() {}
  explicit Post(ARequest* _msg) : msg(std::unique_ptr<ARequest>(_msg)) {}

  explicit Post(std::unique_ptr<ARequest> _msg) : msg(std::move(_msg)) {}
  void send(const Host& host) KTHROW(Exception) {
    send(host.host(), "res", host.port(), {{"session", host.session_id()}});
  }
  void send(std::string const& host, std::string const& res, uint16_t const& port,
            const std::unordered_map<std::string, std::string> headers = {{}})
      KTHROW(maiken::Exception);
  ARequest* message() { return msg.get(); }

  std::string const& body() { return _body; }

  void release() { msg.release(); }

 private:
  Post() {}
  Post(const Post&) = delete;
  Post(const Post&&) = delete;
  Post& operator=(const Post&) = delete;
  Post& operator=(const Post&&) = delete;
  template <class Archive>
  void serialize(Archive& ar) {
    ar(::cereal::make_nvp("msg", msg));
  }

 private:
  std::string _body;
  std::unique_ptr<ARequest> msg = nullptr;
};

class RemoteCommandManager {
 public:
  static RemoteCommandManager& INST() {
    static RemoteCommandManager inst;
    return inst;
  }

  std::unique_ptr<SetupRequest> build_setup_query(maiken::Application const& a,
                                                  mkn::kul::cli::Args const& args);

  std::unique_ptr<CompileRequest> build_compile_request(
      std::string const& directory,
      const std::vector<std::pair<std::string, std::string>>& src_obj);

  std::unique_ptr<DownloadRequest> build_download_request();

  std::unique_ptr<LinkRequest> build_link_request(std::string const& b);

  void build_hosts(const Settings& settings) KTHROW(mkn::kul::Exception) {
    if (settings.root()["dist"]) {
      if (settings.root()["dist"]["nodes"]) {
        for (auto const& node : settings.root()["dist"]["nodes"]) {
          m_hosts.emplace_back(node["host"].Scalar(), mkn::kul::String::UINT16(node["port"].Scalar()));
        }
      }
    }
  }
  const std::vector<Host>& hosts() const { return m_hosts; }

 private:
  std::vector<Host> m_hosts;
};
using RMC = RemoteCommandManager;
}  // end namespace dist
}  // end namespace maiken

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::AMessage, cereal::specialization::member_serialize)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::ARequest, cereal::specialization::member_serialize)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::SetupRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::SetupRequest)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::CompileRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::CompileRequest)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::DownloadRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::DownloadRequest)

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(maiken::dist::LinkRequest,
                                   cereal::specialization::member_serialize)
CEREAL_REGISTER_TYPE(maiken::dist::LinkRequest)

#endif  // _MKN_WITH_MKN_RAM_ && _MKN_WITH_IO_CEREAL_
#endif  // _MAIKEN_DIST_HPP_
