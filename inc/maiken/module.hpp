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
#ifndef MAIKEN_MODULE_HPP
#define MAIKEN_MODULE_HPP

// Module support (dlopen/dlsym-based loading) is opt-in: it only compiles in
// when maiken itself is built "-w mkn.mod", which defines MKN_WITH_MKN_MOD.
#if !MKN_WITH_MKN_MOD
#define MKN_WITH_MKN_MOD 0
#endif

#if MKN_WITH_MKN_MOD

#include "mkn/kul/env.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/sys.hpp"

#include "mkn/mod/def.hpp"
#include "mkn/mod/loader.hpp"

#include "maiken/app.hpp"

namespace maiken {

// Prepares whatever this process needs so ap's module (and its dependency
// libs) can be dlopen()'d/LoadLibrary()'d successfully, and returns an RAII
// guard scoping that preparation to the caller's load call:
//  - Windows: LoadLibrary re-reads PATH per call, so this pushes PATH (+ any
//    of ap's own declared env vars) for the guard's lifetime.
//  - nix: dlopen()'s dynamic linker only reads LD_LIBRARY_PATH once at
//    process startup, so mutating it here would do nothing - instead this
//    preloads ap's dependency .so files by absolute path (dlopen resolves a
//    NEEDED entry against an already-mapped object by soname before ever
//    consulting search paths), and returns a no-op guard.
mkn::kul::env::PushEnv prepareModuleLoad(Application& ap) KTHROW(mkn::kul::sys::Exception);

class GlobalModules;
class MKN_KUL_PUBLISH ModuleLoader : public mkn::mod::Loader {
  friend class GlobalModules;

 private:
  Application const* ap_ = nullptr;

  static mkn::kul::File FIND(Application& a) KTHROW(mkn::kul::sys::Exception);

 public:
  ModuleLoader(Application const& ap, mkn::kul::File const& f) KTHROW(mkn::kul::sys::Exception)
      : mkn::mod::Loader(f), ap_(&ap) {}
  ~ModuleLoader() {
    if (loaded()) KERR << "WARNING: ModuleLoader not unloaded, possible memory leak";
  }
  Application const* app() const { return ap_; }

  static std::shared_ptr<ModuleLoader> LOAD(Application& ap) KTHROW(mkn::kul::sys::Exception);
};

class GlobalModules {
  friend class ModuleLoader;

 private:
  static GlobalModules& INSTANCE() {
    static GlobalModules i;
    return i;
  }
  mkn::kul::hash::map::S2T<std::shared_ptr<mkn::kul::sys::SharedLibrary>> libs;

  ~GlobalModules() { libs.clear(); }
  void load(Application& ap) KTHROW(mkn::kul::sys::Exception) {
    auto guard = prepareModuleLoad(ap);
    auto lib = std::make_shared<mkn::kul::sys::SharedLibrary>(ModuleLoader::FIND(ap));
    libs.insert(std::make_pair(lib->file().dir().real(), lib));
  }
};

}  // namespace maiken

#endif  // MKN_WITH_MKN_MOD
#endif  /* MAIKEN_MODULE_HPP */
