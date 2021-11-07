/**
Copyright (c) 2017, Philip Deegan.
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must exit_codeain the above copyright
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
#include "mkn/kul/cli.hpp"
#include "mkn/kul/signal.hpp"
#include "maiken/dist.hpp"

#if defined(_WIN32) && !defined(bzero)
#define bzero ZeroMemory
#endif

int main(int argc, char* argv[]) {
  mkn::kul::Signal sig;
  int exit_code = 0;
  try {
    using namespace mkn::kul::cli;
    mkn::kul::Dir d = mkn::kul::user::home(mkn::kul::Dir::JOIN(maiken::Constants::STR_MAIKEN, "server"));
    Args args({}, {Arg('d', maiken::Constants::STR_DIR, ArgType::STRING)});
    try {
      args.process(argc, argv);
    } catch (const mkn::kul::cli::Exception& e) {
      KEXIT(1, e.what());
    }
    if (args.has(maiken::Constants::STR_DIR)) {
      d = mkn::kul::Dir(args.get(maiken::Constants::STR_DIR));
      if (!d && !d.mk())
        KEXCEPT(mkn::kul::Exception, "diretory provided does not exist or cannot be created");
    }
    maiken::dist::Server serv(8888, d, 3);
    mkn::kul::Thread thread(std::ref(serv));
    sig.intr([&](int16_t) {
      KERR << "Interrupted";
      thread.interrupt();
      exit(2);
    });
    thread.join();
  } catch (mkn::kul::Exit const& e) {
    if (e.code() != 0) KERR << mkn::kul::os::EOL() << "ERROR: " << e;
    exit_code = e.code();
  } catch (const mkn::kul::proc::ExitException& e) {
    exit_code = e.code();
  } catch (mkn::kul::Exception const& e) {
    KERR << e.stack();
    exit_code = 2;
  } catch (const std::exception& e) {
    KERR << e.what();
    exit_code = 3;
  }
  return exit_code;
}
