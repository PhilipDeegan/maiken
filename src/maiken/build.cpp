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

void
maiken::Application::link(const kul::hash::set::String& objects)
  KTHROW(kul::Exception)
{
  showConfig();
  if (objects.size() > 0) {
    buildDir().mk();
    if (!main.empty())
      buildExecutable(objects);
    else
      buildLibrary(objects);
  } else
    KEXIT(1, "No link objects found, try compile or build.");
}


void
maiken::Application::checkErrors(const CompilerProcessCapture& cpc)
  KTHROW(kul::Exception)
{
  auto o = [](const std::string& s) {
    if (s.size())
      KOUT(NON) << s;
  };
  auto e = [](const std::string& s) {
    if (s.size())
      KERR << s;
  };
  if (kul::LogMan::INSTANCE().inf() || cpc.exception())
    o(cpc.outs());
  if (kul::LogMan::INSTANCE().err() || cpc.exception())
    e(cpc.errs());
  if (cpc.exception())
    std::rethrow_exception(cpc.exception());
}
