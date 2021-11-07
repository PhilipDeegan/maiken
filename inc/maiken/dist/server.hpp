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
#ifndef _MAIKEN_DIST_SERVER_HPP_
#define _MAIKEN_DIST_SERVER_HPP_

namespace maiken {
namespace dist {

class FileWriter {
 public:
  std::unique_ptr<mkn::kul::io::BinaryWriter> bw;
};

enum class SessionState : uint16_t { NON = 0, ONE, TWO };

class Server;
class ServerSession {
  friend class Server;

 public:
  ServerSession() : start_time(mkn::kul::Now::MILLIS()) {}
  void reset_setup(SetupRequest* request) { setup.reset(request); }
  SetupRequest* setup_ptr() { return setup.get(); }
  void set_apps(const std::vector<Application*>& _apps) { this->apps = std::move(_apps); }
  std::vector<Application*> apps_vector() { return apps; };

 public:
  std::unique_ptr<mkn::kul::io::BinaryReader> binary_reader;
  std::unique_ptr<mkn::kul::io::BinaryWriter> binary_writer;
  std::vector<std::pair<std::string, std::string>> m_src_obj;
  mkn::kul::hash::set::String objects;

 private:
  uint64_t start_time;
  std::unique_ptr<SetupRequest> setup = nullptr;
  std::vector<Application*> apps;
  SessionState state = SessionState::NON;
};

class Server : public mkn::kul::http::MultiServer, public Constants {
  friend class mkn::kul::Thread;

 public:
  Server(uint16_t const port, const mkn::kul::Dir& _home, uint16_t threads)
      : mkn::kul::http::MultiServer(port, 1, threads), busy(false), m_home(_home) {}
  virtual ~Server() {}
  mkn::kul::http::_1_1Response respond(const mkn::kul::http::A1_1Request& req) override;

  Server(const Server&) = delete;
  Server(const Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(const Server&&) = delete;

 protected:
  void onConnect(char const* cip, uint16_t const& port) override {
    std::string ip(cip);
    if (!sessions.count(std::string(ip))) sessions.emplace();
    ServerSession& sesh = sessions[ip];
    if (sesh.state != SessionState::NON && sesh.setup == nullptr) {
      KLOG(ERR) << "Incoming connection has invalid session state - resetting";
      sessions.erase(ip);
      onConnect(cip, port);
    }
  }

 private:
  void operator()();

 private:
  std::atomic<bool> busy;
  mkn::kul::Dir m_home;
  FileWriter fw;
  std::unordered_map<std::string, ServerSession> sessions;
};
}  // end namespace dist
}  // end namespace maiken

#endif  // _MAIKEN_DIST_SERVER_HPP_
