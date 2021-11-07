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
#include "maiken.hpp"

void maiken::Application::link(mkn::kul::hash::set::String const& objects) KTHROW(mkn::kul::Exception) {
  showConfig();
  if (objects.size() > 0 || main_) {
    buildDir().mk();
    if (main_)
      buildExecutable(objects);
    else
      buildLibrary(objects);
    mkn::kul::os::PushDir pushd(this->project().dir());
    mkn::kul::Dir build(".mkn/build");
    build.mk();
    mkn::kul::File ts("timestamp", build);
    if (ts) ts.rm();
    mkn::kul::io::Writer(ts) << mkn::kul::Now::MILLIS();
  }
  if (CommandStateMachine::INSTANCE().commands().count(STR_TEST) && !tests.empty())
    buildTest(objects);

  auto delEmpty = [](auto dir) {
    if (dir && dir.files().empty()) dir.rm();
  };

  delEmpty(mkn::kul::Dir(".mkn/log/" + buildDir().name() + "/bin/cmd"));
  delEmpty(mkn::kul::Dir(".mkn/log/" + buildDir().name() + "/bin/out"));
  delEmpty(mkn::kul::Dir(".mkn/log/" + buildDir().name() + "/bin/err"));
}

void maiken::Application::checkErrors(CompilerProcessCapture const& cpc) KTHROW(mkn::kul::Exception) {
  auto o = [](std::string const& s) {
    if (s.size()) KOUT(NON) << s;
  };
  auto e = [](std::string const& s) {
    if (s.size()) KERR << s;
  };
  if (mkn::kul::LogMan::INSTANCE().inf() || cpc.exception()) o(cpc.outs());
  if (mkn::kul::LogMan::INSTANCE().err() || cpc.exception()) e(cpc.errs());
  if (cpc.exception()) std::rethrow_exception(cpc.exception());
}

bool maiken::Application::is_build_required() {
  mkn::kul::os::PushDir pushd(this->project().dir());
  mkn::kul::Dir bDir(".mkn/build");
  return !bDir || bDir.files().size() == 0 || buildDir().dirs().size() == 0 ||
         buildDir().files().size() == 0;
}

bool maiken::Application::is_build_stale() {
  mkn::kul::os::PushDir pushd(this->project().dir());
  mkn::kul::Dir d(".mkn/build");
  mkn::kul::File f("timestamp", d);
  if (!d || !f) return true;
  size_t then = size_t{43200} * size_t{60} * size_t{1000};
  size_t now = mkn::kul::Now::MILLIS();
  size_t timestamp = mkn::kul::String::UINT64(std::string(mkn::kul::io::Reader(f).readLine()));
  if (now - then > timestamp) return true;
  return false;
}
