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

mkn::kul::File maiken::ModuleLoader::FIND(Application& ap)
#ifndef _MKN_DISABLE_MODULES_
    KTHROW(mkn::kul::sys::Exception)
#endif  //_MKN_DISABLE_MODULES_
{
#ifdef _MKN_DISABLE_MODULES_
  KEXCEPT(mkn::kul::Exception, "Modules are disabled: ") << ap.project().dir();
#endif  //_MKN_DISABLE_MODULES_

  std::string file;
  std::vector<mkn::kul::File> files;
  if (ap.buildDir())
    for (auto const& f : ap.buildDir().files(0)) files.emplace_back(f.real());
  else {
    if (!ap.libraries().empty() && !ap.libraryPaths().empty()) {
      for (auto const& path : ap.libraryPaths())
        for (auto const& f : mkn::kul::Dir(path).files(0)) files.emplace_back(f.real());
    }
  }
  if (files.empty() && !ap.sources().empty()) {
    ap.process();
    if (ap.buildDir())
      for (auto const& f : ap.buildDir().files(0)) files.emplace_back(f.real());
  }
  for (auto const& f : files) {
    auto const& name(f.name());
    if (name.find(".") != std::string::npos &&
        name.find(ap.project().root()["name"].Scalar()) != std::string::npos
#ifdef _WIN32
        && name.substr(name.rfind(".") + 1) == "dll") {
#else
        && name.substr(name.rfind(".") + 1) == "so") {
#endif
      file = f.real();
      break;
    }
  }
  mkn::kul::File lib(file);
#ifndef _MKN_DISABLE_MODULES_
  if (!lib)
    KEXCEPT(mkn::kul::sys::Exception, "No loadable library found for project: ") << ap.project().dir();
#endif  //_MKN_DISABLE_MODULES_
  return lib;
}

std::shared_ptr<maiken::ModuleLoader> maiken::ModuleLoader::LOAD(Application& ap)
#ifndef _MKN_DISABLE_MODULES_
    KTHROW(mkn::kul::sys::Exception)
#endif  //_MKN_DISABLE_MODULES_
{
  std::function<void(Application & ap)> global_load = [&](Application& ap) {
    for (auto dep = ap.dependencies().rbegin(); dep != ap.dependencies().rend(); ++dep) {
      auto& dap = (**dep);
      if (!dap.sources().empty()) GlobalModules::INSTANCE().load(dap);
      global_load(dap);
    }
  };
  global_load(ap);

  return std::make_shared<ModuleLoader>(ap, FIND(ap));
}
