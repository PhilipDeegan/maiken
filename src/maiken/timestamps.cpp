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

void maiken::Application::writeTimeStamps(mkn::kul::hash::set::String& objects,
                                          std::vector<mkn::kul::File>& cacheFiles) {
  std::string const oType("." + (*AppVars::INSTANCE().envVars().find("MKN_OBJ")).second);
  mkn::kul::Dir mkn(buildDir().join(".mkn"), 1);
  mkn::kul::File srcStamps("src_stamp", mkn);
  mkn::kul::File incStamps("inc_stamp", mkn);
  for (auto const& src : stss)
    if (std::find(cacheFiles.begin(), cacheFiles.end(), src.first) == cacheFiles.end())
      cacheFiles.push_back(src.first);
  mkn::kul::hash::map::S2T<Compiler const*> compilers;
  for (auto const& f : cacheFiles) {
    std::string ft = f.name().substr(f.name().rfind(".") + 1);
    Compiler const* compiler;
    if (compilers.count(ft))
      compiler = (*compilers.find(ft)).second;
    else {
      compiler = Compilers::INSTANCE().get((*(*files().find(ft)).second.find(STR_COMPILER)).second);
      compilers.insert(ft, compiler);
    }
    if (compiler->sourceIsBin())
      if (std::find(objects.begin(), objects.end(), f.escm()) == objects.end())
        objects.insert(f.escm());
  }
  for (auto const& f : buildDir().files(1))
    if (f.name().size() > 4 && f.name().substr(f.name().size() - 4) == oType)
      if (std::find(objects.begin(), objects.end(), f.escm()) == objects.end())
        objects.insert(f.escm());

  mkn::kul::io::Writer srcW(srcStamps);
  mkn::kul::io::Writer incW(incStamps);
  for (auto const& i : includeStamps) incW << i.first << " " << i.second << mkn::kul::os::EOL();
  for (auto const& src : cacheFiles)
    if (!(*compilers.find(src.name().substr(src.name().rfind(".") + 1))).second->sourceIsBin())
      srcW << src.mini() << " " << src.timeStamps().modified() << mkn::kul::os::EOL();
}

void maiken::Application::loadTimeStamps() KTHROW(mkn::kul::StringException) {
  if (_MKN_TIMESTAMPS_) {
    mkn::kul::Dir mkn(buildDir().join(".mkn"));
    mkn::kul::File src("src_stamp", mkn);
    if (mkn && src) {
      mkn::kul::io::Reader r(src);
      char const* c = 0;
      while ((c = r.readLine())) {
        std::string s(c);
        if (s.size() == 0) continue;
        std::vector<std::string> bits;
        mkn::kul::String::SPLIT(s, ' ', bits);
        if (bits.size() != 2) KEXIT(1, "timestamp file invalid format\n" + src.full());
        try {
          stss.insert(bits[0], mkn::kul::String::UINT64(bits[1]));
        } catch (const mkn::kul::StringException& e) {
          KEXIT(1, "timestamp file invalid format\n" + src.full());
        }
      }
    }
    mkn::kul::File inc("inc_stamp", mkn);
    if (mkn && inc) {
      mkn::kul::io::Reader r(inc);
      char const* c = 0;
      while ((c = r.readLine())) {
        std::string s(c);
        if (s.size() == 0) continue;
        std::vector<std::string> bits;
        mkn::kul::String::SPLIT(s, ' ', bits);
        if (bits.size() != 2) KEXIT(1, "timestamp file invalid format\n" + inc.full());
        itss.insert(bits[0], bits[1]);
      }
    }
    for (auto const& i : includes()) {
      mkn::kul::Dir inc(i.first);
      uint64_t includeStamp = inc.timeStamps().modified();
      for (auto const& fi : inc.files(1)) includeStamp += fi.timeStamps().modified();
      std::ostringstream os;
      os << std::hex << includeStamp;
      includeStamps.insert(inc.mini(), os.str());
    }
  }
}