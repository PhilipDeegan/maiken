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

#if MKN_WITH_MKN_MOD

mkn::kul::File maiken::ModuleLoader::FIND(Application& ap) KTHROW(mkn::kul::sys::Exception) {
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

  auto const mod_name = ap.baseLibFilename();

  for (auto const& f : files) {
    auto const& name(f.name());
    if (name.find(".") != std::string::npos && name.find(mod_name) != std::string::npos
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
  if (!lib)
    KEXCEPT(mkn::kul::sys::Exception, "No loadable library found for project: ",
            ap.project().dir());
  return lib;
}

std::shared_ptr<maiken::ModuleLoader> maiken::ModuleLoader::LOAD(Application& ap)
    KTHROW(mkn::kul::sys::Exception) {
  std::function<void(Application & ap)> global_load = [&](Application& ap) {
    for (auto dep = ap.moduleDependencies().rbegin(); dep != ap.moduleDependencies().rend();
         ++dep) {
      auto& dap = (**dep);
      if (!dap.sources().empty()) GlobalModules::INSTANCE().load(dap);
      global_load(dap);
    }
  };
  global_load(ap);

  auto guard = prepareModuleLoad(ap);
  return std::make_shared<ModuleLoader>(ap, FIND(ap));
}

#ifndef _WIN32
namespace {
// dlopen()'s dynamic linker resolves a NEEDED entry by soname against
// already-mapped objects before ever consulting search paths, so loading
// ap's dependency .so files by absolute path first is enough to satisfy the
// module's NEEDED entries for them, without touching LD_LIBRARY_PATH at all.
// Kept alive process-lifetime (same rationale as GlobalModules::libs).
mkn::kul::hash::map::S2T<std::shared_ptr<mkn::kul::sys::SharedLibrary>>& PreloadedDeps() {
  static mkn::kul::hash::map::S2T<std::shared_ptr<mkn::kul::sys::SharedLibrary>> libs;
  return libs;
}

void PreloadDependencies(maiken::Application const& ap) {
  auto& preloaded = PreloadedDeps();
  for (auto const& dir : ap.libraryPaths()) {
    mkn::kul::Dir d(dir);
    if (!d) continue;
    for (auto const& f : d.files(0)) {
      auto const& name = f.name();
      auto const dot = name.rfind(".");
      if (dot == std::string::npos || name.substr(dot + 1) != "so") continue;
      auto const real = f.real();
      if (preloaded.count(real)) continue;
      try {
        preloaded.insert(std::make_pair(
            real, std::make_shared<mkn::kul::sys::SharedLibrary>(mkn::kul::File(real))));
        KLOG(TRC) << "preloaded module dependency: " << real;
      } catch (mkn::kul::sys::Exception const& e) {
        KLOG(TRC) << "could not preload module dependency: " << real << " : " << e.what();
      }
    }
  }
}
}  // namespace
#endif  // _WIN32

mkn::kul::env::PushEnv maiken::prepareModuleLoad(Application& ap) KTHROW(mkn::kul::sys::Exception) {
#ifdef _WIN32
  auto vars = ap.requiredEnv();
  for (auto const& v : ap.envVars()) vars.push_back(v);
  return mkn::kul::env::PushEnv(vars);
#else
  if (Settings::INSTANCE().propertyBool("mkn.env.automatic")) PreloadDependencies(ap);
  return mkn::kul::env::PushEnv(std::vector<mkn::kul::env::Var>{});
#endif
}

#endif  // MKN_WITH_MKN_MOD
