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

void maiken::Application::findObjects(kul::hash::set::String &objects) const {
  for (const auto &ft : sourceMap()) {
    try {
      if (!(*files().find(ft.first)).second.count(STR_COMPILER))
        KEXIT(1, "No compiler found for filetype " + ft.first);
      const auto *compiler =
          Compilers::INSTANCE().get((*(*files().find(ft.first)).second.find(STR_COMPILER)).second);
      if (!compiler->sourceIsBin()) {
        if (!buildDir().is())
          KEXCEPT(maiken::Exception, "Cannot link without compiling.\n" + project().dir().path());
        kul::Dir objDir("obj", buildDir());
        if (!buildDir().is())
          KEXCEPT(maiken::Exception, "No object directory found.\n" + project().dir().path());
        for (const kul::File f : objDir.files(true)) objects.insert(f.real());
      } else {
        for (const auto &kv : ft.second)
          for (const auto &f : kv.second) objects.insert(kul::File(f.in()).mini());
      }
    } catch (const CompilerNotFoundException &e) {
      KEXIT(1, "No compiler found for filetype " + ft.first);
    }
  }
}
