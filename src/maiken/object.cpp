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

void maiken::Application::findObjects(mkn::kul::hash::set::String& objects) const {
  for (auto const& ft : sourceMap()) {
    try {
      if (!(*files().find(ft.first)).second.count(STR_COMPILER))
        KEXIT(1, "No compiler found for filetype " + ft.first);
      auto compiler = Compilers::INSTANCE().get(files().at(ft.first).at(STR_COMPILER));
      if (!compiler->sourceIsBin()) {
        if (!buildDir().is())
          KEXCEPT(maiken::Exception, "Cannot link without compiling.\n" + project().dir().path());
        mkn::kul::Dir objDir("obj", buildDir());
        if (!buildDir().is())
          KEXCEPT(maiken::Exception, "No object directory found.\n" + project().dir().path());
        std::string const oType = "." + AppVars::INSTANCE().envVars().at("MKN_OBJ");
        for (mkn::kul::File const& f : objDir.files(true)) {
          auto file = f.mini();
          if (file.rfind(oType) == file.size() - oType.size()) objects.insert(file);
        }
      } else {
        for (auto const& kv : ft.second)
          for (auto const& f : kv.second) objects.insert(mkn::kul::File(f.in()).mini());
      }
    } catch (CompilerNotFoundException const& e) {
      KEXIT(1, "No compiler found for filetype " + ft.first);
    }
  }
}
