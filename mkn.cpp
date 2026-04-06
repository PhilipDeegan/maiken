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

#include "maiken.hpp"
#include "maiken/defs.hpp"

#include "mkn/kul/log.hpp"
#include "mkn/kul/signal.hpp"

namespace maiken {

int main_entry(int argc, char* argv[]) {
  PROGRAM = argv[0];
  mkn::kul::Signal sig;
  uint8_t ret = 0;
  auto const s = mkn::kul::Now::MILLIS();

  try {
    for (auto app : Application::CREATE(argc, argv)) app->process();

    bool const print_build_time = !AppVars::INSTANCE().dryRun() && CommandStateMachine::has_acted();
    if (print_build_time) {
      KOUT(NON) << "BUILD TIME: " << (mkn::kul::Now::MILLIS() - s) << " ms";
      KOUT(NON) << "FINISHED:   " << mkn::kul::DateTime::NOW();
    }
  } catch (mkn::kul::Exit const& e) {
    if (e.code() != 0) KERR << mkn::kul::os::EOL() << "ERROR: " << e.stack();
    ret = e.code();
  } catch (mkn::kul::proc::ExitException const& e) {
    KERR << e;
    ret = e.code();
  } catch (mkn::kul::Exception const& e) {
    KERR << e.stack();
    ret = 1;
  } catch (std::exception const& e) {
    KERR << e.what();
    ret = 1;
  }
  return ret;
}

}  // namespace maiken

int main(int argc, char* argv[]) { return maiken::main_entry(argc, argv); }
