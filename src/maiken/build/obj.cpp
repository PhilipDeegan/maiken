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
#include "maiken/source.hpp"

#include <mutex>

namespace maiken {
class CompilerPrinter {
 public:
  static void print_for(Application const& app) {
    if (!AppVars::INSTANCE().dryRun()) {
      std::stringstream ss;
      ss << MKN_PROJECT << ": " << app.project().dir().path();
      if (app.profile().size() > 0) ss << " [" << app.profile() << "]";
      KOUT(NON) << ss.str();
    }
    if (!AppVars::INSTANCE().dryRun() && mkn::kul::LogMan::INSTANCE().inf() && app.includes().size()) {
      KOUT(NON) << "INCLUDES";
      for (auto const& s : app.includes()) KOUT(NON) << "\t" << s.first;
    }

    if (!AppVars::INSTANCE().dryRun() && mkn::kul::LogMan::INSTANCE().inf()) {
      if (!app.arg.empty()) KOUT(NON) << "ARGUMENTS\n\t" << app.arg;
      if (app.arguments().size()) {
        KOUT(NON) << "FILE ARGUMENTS";
        for (auto const& kv : app.arguments())
          for (std::string const& s : kv.second) KOUT(NON) << "\t" << kv.first << " : " << s;
      }
      if (!AppVars::INSTANCE().args().empty())
        KOUT(NON) << "ADDITIONAL ARGUMENTS: \n\t" << AppVars::INSTANCE().args();
      if (AppVars::INSTANCE().jargs().size()) {
        KOUT(NON) << "ADDITIONAL FILE ARGUMENTS:";
        for (auto const& kv : AppVars::INSTANCE().jargs())
          KOUT(NON) << "\t" << kv.first << " : " << kv.second;
      }
    }
    if (!AppVars::INSTANCE().envVars().count("MKN_OBJ")) KEXCEPTION("INTERNAL BADNESS ERROR!");
  }
};
}  // namespace maiken

void maiken::Application::compile(mkn::kul::hash::set::String& objects) KTHROW(mkn::kul::Exception) {
  auto sources = sourceMap();

  showConfig();
  CompilerPrinter::print_for(*this);

  SourceFinder s_finder(*this);
  CompilerValidation::check_compiler_for(*this, sources);
  std::vector<mkn::kul::File> cacheFiles;
  auto src_objs = s_finder.all_sources_from(sources, objects, cacheFiles);

  compile(src_objs, objects, cacheFiles);
}

void maiken::Application::compile(std::vector<std::pair<maiken::Source, std::string>>& src_objs,
                                  mkn::kul::hash::set::String& objects,
                                  std::vector<mkn::kul::File>& cacheFiles) KTHROW(mkn::kul::Exception) {
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  std::vector<std::shared_ptr<maiken::dist::Post>> posts;
  auto compile_lambda = [](std::shared_ptr<maiken::dist::Post> post, const dist::Host& host) {
    post->send(host);
    dist::FileWriter fw;
    dist::Blob b;
    do {
      auto dowd = std::make_shared<maiken::dist::Post>(
          maiken::dist::RemoteCommandManager::INST().build_download_request());
      dowd->send(host);
      std::string body(std::move(dowd->body()));
      std::istringstream iss(body);
      {
        cereal::PortableBinaryInputArchive iarchive(iss);
        iarchive(b);
      }
      if (!b.file.empty()) {
        if (!fw.bw) {
          mkn::kul::File obj(b.file);
          if (obj) obj = mkn::kul::File(std::string(b.file + ".new"));
          fw.bw = std::make_unique<mkn::kul::io::BinaryWriter>(obj);
        }
        fw.bw->write(b.c1, b.len);
      }
      if (b.last_packet) fw.bw.reset();
    } while (b.files_left > 0);
  };
  size_t threads = 0;

  auto& hosts(maiken::dist::RemoteCommandManager::INST().hosts());
  if (AppVars::INSTANCE().nodes()) {
    threads =
        (hosts.size() < AppVars::INSTANCE().nodes()) ? hosts.size() : AppVars::INSTANCE().nodes();
  }
  mkn::kul::ChroncurrentThreadPool<> ctp(threads, 1, 1000000000, 1000);
  auto compile_ex = [&](mkn::kul::Exception const& e) {
    ctp.stop().interrupt();
    throw e;
  };
  {
    size_t sources = src_objs.size();
    size_t divisor = std::floor(sources / (threads + 1));
    for (size_t i = 0; i < threads; i++) {
      std::vector<std::pair<std::string, std::string>> remote_src_objs;
      for (size_t k = 0; k < divisor; k++) {
        remote_src_objs.push_back(src_objs[0]);
        src_objs.erase(src_objs.begin());
      }

      if (!remote_src_objs.empty()) {
        posts.emplace_back(std::make_shared<maiken::dist::Post>(
            maiken::dist::RemoteCommandManager::INST().build_compile_request(
                this->project().dir().real(), remote_src_objs)));
        ctp.async(std::bind(compile_lambda, posts[i], std::ref(hosts[i])), compile_ex);
      }
    }
  }

#endif  //  _MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  std::queue<std::pair<maiken::Source, std::string>> sourceQueue;
  for (auto& so : src_objs) {
    sourceQueue.emplace(so.first, so.second);
    mkn::kul::File object_file(so.second);
    if (!object_file.dir()) object_file.dir().mk();
  }
  if (!src_objs.empty()) compile(sourceQueue, objects, cacheFiles);
#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  ctp.finish(50000000);  // 50 milliseconds
  ctp.rethrow();
#endif  //  _MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)

  if (_MKN_TIMESTAMPS_) writeTimeStamps(objects, cacheFiles);
}

void maiken::Application::compile(std::queue<std::pair<maiken::Source, std::string>>& sourceQueue,
                                  mkn::kul::hash::set::String& objects,
                                  std::vector<mkn::kul::File>& cacheFiles) KTHROW(mkn::kul::Exception) {
  ThreadingCompiler tc(*this);
  mkn::kul::ChroncurrentThreadPool<> ctp(AppVars::INSTANCE().threads(), 1, 1000000000, 1000);
  std::vector<maiken::CompilationUnit> c_units;
  std::queue<std::pair<maiken::Source, std::string>> cQueue;

  while (sourceQueue.size() > 0) {
    cQueue.push(sourceQueue.front());
    c_units.emplace_back(tc.compilationUnit(sourceQueue.front()));
    sourceQueue.pop();
  }

  auto o = [](std::string const& s) {
    if (s.size()) KOUT(NON) << s;
  };
  auto e = [](std::string const& s) {
    if (s.size()) KERR << s;
  };

  std::mutex mute;
  std::vector<CompilerProcessCapture> cpcs;

  auto lambex = [&](mkn::kul::Exception const&) {
    ctp.stop();
    ctp.interrupt();
  };

  mkn::kul::Dir cmdLogDir(".mkn/log/" + buildDir().name() + "/obj/cmd", 1);
  mkn::kul::Dir outLogDir(".mkn/log/" + buildDir().name() + "/obj/out", 1);
  mkn::kul::Dir errLogDir(".mkn/log/" + buildDir().name() + "/obj/err", 1);

  auto lambda = [&, o, e](const maiken::CompilationUnit& c_unit) {
    CompilerProcessCapture const cpc = c_unit.compile();
    if (!AppVars::INSTANCE().dryRun()) {
      if (mkn::kul::LogMan::INSTANCE().inf() || cpc.exception()) o(cpc.outs());
      if (mkn::kul::LogMan::INSTANCE().inf() || cpc.exception()) e(cpc.errs());
      KOUT(INF) << cpc.cmd();
    } else
      KOUT(NON) << cpc.cmd();

    if (AppVars::INSTANCE().dump()) {
      std::string base = mkn::kul::File(cpc.file()).name();
      mkn::kul::io::Writer(mkn::kul::File(base + ".txt", cmdLogDir)) << cpc.cmd();
      if (cpc.outs().size()) mkn::kul::io::Writer(mkn::kul::File(base + ".txt", outLogDir)) << cpc.outs();
      if (cpc.errs().size()) mkn::kul::io::Writer(mkn::kul::File(base + ".txt", errLogDir)) << cpc.errs();
    }

    std::lock_guard<std::mutex> lock(mute);
    cpcs.push_back(cpc);

    try {
      if (!AppVars::INSTANCE().force())
        if (cpc.exception()) std::rethrow_exception(cpc.exception());

    } catch (mkn::kul::Exception const& e) {
      lambex(e);
    } catch (const std::exception& e) {
      KLOG(ERR) << e.what();
    }
  };

  for (auto const& unit : c_units) {
    mkn::kul::this_thread::nSleep(5000000);  // dup appears to be overloaded with too many threads
    ctp.async(std::bind(lambda, unit), std::bind(lambex, std::placeholders::_1));
  }

  ctp.finish(1000000 * 1000);

  auto delEmpty = [](auto& dir) {
    if (dir.files().empty()) dir.rm();
  };

  delEmpty(cmdLogDir);
  delEmpty(outLogDir);
  delEmpty(errLogDir);

  if (!AppVars::INSTANCE().force())
    if (ctp.exception()) KEXIT(1, "Compile error detected");

  if (!AppVars::INSTANCE().force())
    for (auto& cpc : cpcs)
      if (cpc.exception()) std::rethrow_exception(cpc.exception());

  mkn::kul::Dir tmpD(buildDir().join("tmp"), 1);
  while (cQueue.size()) {
    mkn::kul::Dir dir(mkn::kul::File(cQueue.front().second).dir());
    if (dir.real() != tmpD.real()) {
      objects.insert(cQueue.front().second);
      cacheFiles.emplace_back(mkn::kul::File(cQueue.front().first.in()));
    }
    cQueue.pop();
  }
}
