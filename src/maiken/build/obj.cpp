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
#include "maiken/dist.hpp"

#include <mutex>

namespace maiken {
class CompilerPrinter {
 public:
  void print_for(const Application& app) const {
    if (!AppVars::INSTANCE().dryRun()) {
      std::stringstream ss;
      ss << MKN_PROJECT << ": " << app.project().dir().path();
      if (app.profile().size() > 0) ss << " [" << app.profile() << "]";
      KOUT(NON) << ss.str();
    }
    if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf() &&
        app.includes().size()) {
      KOUT(NON) << "INCLUDES";
      for (const auto& s : app.includes()) KOUT(NON) << "\t" << s.first;
    }
    if (!AppVars::INSTANCE().dryRun() && app.srcs.empty() && app.main.empty()) {
      KOUT(NON) << "NO SOURCES";
      return;
    }

    if (!AppVars::INSTANCE().dryRun() && kul::LogMan::INSTANCE().inf()) {
      if (!app.arg.empty()) KOUT(NON) << "ARGUMENTS\n\t" << app.arg;
      if (app.arguments().size()) {
        KOUT(NON) << "FILE ARGUMENTS";
        for (const auto& kv : app.arguments())
          for (const std::string& s : kv.second)
            KOUT(NON) << "\t" << kv.first << " : " << s;
      }
      if (!AppVars::INSTANCE().args().empty())
        KOUT(NON) << "ADDITIONAL ARGUMENTS: \n\t" << AppVars::INSTANCE().args();
      if (AppVars::INSTANCE().jargs().size()) {
        KOUT(NON) << "ADDITIONAL FILE ARGUMENTS:";
        for (const auto& kv : AppVars::INSTANCE().jargs())
          KOUT(NON) << "\t" << kv.first << " : " << kv.second;
      }
    }
    if (!AppVars::INSTANCE().envVars().count("MKN_OBJ"))
      KEXCEPTION("INTERNAL BADNESS ERROR!");
  }
};
}  // namespace maiken

void maiken::Application::compile(kul::hash::set::String& objects)
    KTHROW(kul::Exception) {
  auto sources = sourceMap();

  showConfig();
  CompilerPrinter().print_for(*this);

  SourceFinder s_finder(*this);
  CompilerValidation::check_compiler_for(*this, sources);
  std::vector<kul::File> cacheFiles;
  auto src_objs = s_finder.all_sources_from(sources, objects, cacheFiles);

  compile(src_objs, objects, cacheFiles);
}

void maiken::Application::compile(
    std::vector<std::pair<std::string, std::string>>& src_objs,
    kul::hash::set::String& objects, std::vector<kul::File>& cacheFiles)
    KTHROW(kul::Exception) {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  std::vector<std::thread> wait_for_compile(0);
  auto compile_lambda = [](std::shared_ptr<maiken::dist::Post>& post) {
    try {
      post->send();
      dist::FileWriter fw;
      dist::Blob b;
      do {
        auto dowd = std::make_shared<maiken::dist::Post>(
            maiken::dist::RemoteCommandManager::INST()
                .build_download_request());
        dowd->send();
        std::string body(std::move(dowd->body()));
        std::istringstream iss(body);
        {
          cereal::PortableBinaryInputArchive iarchive(iss);
          iarchive(b);
        }
        size_t size = b.len;
        if (!b.file.empty()) {
          if (!fw.bw) {
            kul::File obj(b.file);
            if (obj) obj = kul::File(std::string(b.file + ".new"));
            fw.bw = std::make_unique<kul::io::BinaryWriter>(obj);
          }
          for (size_t i = 0; i < b.len; i++) (*fw.bw.get()) << b.c1[i];
        }
        if (b.last_packet) fw.bw.reset();
      } while (b.files_left > 0);
    } catch (const kul::Exception& e) {
      KLOG(ERR) << e.stack();
    } catch (const std::exception& e) {
      KLOG(ERR) << e.what();
    } catch (...) {
      KLOG(ERR) << "negotiate.send() ERROR!";
    }
  };
  std::shared_ptr<maiken::dist::Post> post;
  if (AppVars::INSTANCE().nodes()) {
    std::vector<std::pair<std::string, std::string>> remote_src_objs;
    {
      remote_src_objs.push_back(src_objs[0]);
      src_objs.erase(src_objs.begin());
    }
    post = std::make_shared<maiken::dist::Post>(
        maiken::dist::RemoteCommandManager::INST().build_compile_request(
            this->project().dir().real(),
            remote_src_objs));
    wait_for_compile.emplace_back(compile_lambda, std::ref(post));
  }
  wait_for_compile.emplace_back([&]() {
#endif  //  _MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
    std::queue<std::pair<std::string, std::string>> sourceQueue;
    for (auto& so : src_objs) {
      sourceQueue.push(std::make_pair(so.first, so.second));
      kul::File object_file(so.second);
      if (!object_file.dir()) object_file.dir().mk();
    }
    if (!src_objs.empty()) compile(sourceQueue, objects, cacheFiles);
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  });
  for (auto& thread : wait_for_compile) {
    thread.join();
  }
#endif  //  _MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

  if (_MKN_TIMESTAMPS_) writeTimeStamps(objects, cacheFiles);
}

void maiken::Application::compile(
    std::queue<std::pair<std::string, std::string>>& sourceQueue,
    kul::hash::set::String& objects, std::vector<kul::File>& cacheFiles)
    KTHROW(kul::Exception) {
  ThreadingCompiler tc(*this);
  kul::ChroncurrentThreadPool<> ctp(AppVars::INSTANCE().threads(), 1,
                                    1000000000, 1000);
  std::vector<maiken::CompilationUnit> c_units;
  std::queue<std::pair<std::string, std::string>> cQueue;
  while (sourceQueue.size() > 0) {
    c_units.emplace_back(tc.compilationUnit(sourceQueue.front()));
    cQueue.push(sourceQueue.front());
    sourceQueue.pop();
  }

  auto o = [](const std::string& s) {
    if (s.size()) KOUT(NON) << s;
  };
  auto e = [](const std::string& s) {
    if (s.size()) KERR << s;
  };

  std::mutex mute;
  std::vector<CompilerProcessCapture> cpcs;
  auto lambda = [o, e, &ctp, &mute,
                 &cpcs](const maiken::CompilationUnit& c_unit) {
    if (ctp.exception()) ctp.interrupt();
    const CompilerProcessCapture cpc = c_unit.compile();
    if (!AppVars::INSTANCE().dryRun()) {
      if (kul::LogMan::INSTANCE().inf() || cpc.exception()) o(cpc.outs());
      if (kul::LogMan::INSTANCE().inf() || cpc.exception()) e(cpc.errs());
      KOUT(INF) << cpc.cmd();
    } else
      KOUT(NON) << cpc.cmd();
    std::lock_guard<std::mutex> lock(mute);
    cpcs.push_back(cpc);
  };

  auto lambex = [&](const kul::Exception& e) {
    ctp.stop();
    std::cerr << ":EXCEPTION!: " << e.debug() << std::endl;
    throw e;
  };

  for (const auto& unit : c_units) {
    kul::this_thread::nSleep(
        5000000);  // dup appears to be overloaded with too many threads
    ctp.async(std::bind(lambda, unit),
              std::bind(lambex, std::placeholders::_1));
  }

  ctp.finish(1000000 * 1000);
  if (ctp.exception()) KEXIT(1, "Compile error detected");

  for (auto& cpc : cpcs) {
    if (cpc.exception()) std::rethrow_exception(cpc.exception());
  }

  while (cQueue.size()) {
    objects.insert(cQueue.front().second);
    cacheFiles.push_back(kul::File(cQueue.front().first));
    cQueue.pop();
  }
}
