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
#ifndef _MAIKEN_SOURCE_HPP_
#define _MAIKEN_SOURCE_HPP_

#include "maiken/app.hpp"

namespace maiken {

class Source {
 public:
  explicit Source(std::string in) : m_in(in) {}
  explicit Source(std::string in, std::string args) : m_in(in), m_args(args) {}
  auto& args() const { return m_args; }
  auto& in() const { return m_in; }

  std::string object() const;

  bool operator==(Source const& that) const { return this->in() == that.in(); }

 private:
  std::string m_in, m_args;
};

class SourceFinder : public Constants {
 public:
  using SourceMap = mkn::kul::hash::map::S2T<mkn::kul::hash::map::S2T<std::vector<maiken::Source>>>;
  SourceFinder(maiken::Application const& _app);
  std::vector<std::pair<maiken::Source, std::string>> all_sources_from(
      SourceMap const& sources, mkn::kul::hash::set::String& objects,
      std::vector<mkn::kul::File>& cacheFiles);

  std::vector<maiken::Source> tests();

 private:
  maiken::Application const& app;
  std::string const oType;
  mkn::kul::Dir objD, tmpD;
};
}  // end namespace maiken

#endif  // _MAIKEN_SOURCE_HPP_
