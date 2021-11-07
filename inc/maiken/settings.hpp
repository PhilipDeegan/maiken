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
#ifndef _MAIKEN_SETTINGS_HPP_
#define _MAIKEN_SETTINGS_HPP_

#include "mkn/kul/os.hpp"
#include "mkn/kul/log.hpp"
#include "mkn/kul/yaml.hpp"

#include "maiken/defs.hpp"

namespace maiken {

class SettingsException : public mkn::kul::Exception {
 public:
  SettingsException(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class Settings : public mkn::kul::yaml::File, public Constants {
 private:
  std::vector<std::string> rrs, rms;
  std::unique_ptr<Settings> sup;
  mkn::kul::hash::map::S2S ps;

  void resolveProperties() KTHROW(SettingsException);
  static std::unique_ptr<Settings> instance;
  static void write(mkn::kul::File const& f) KTHROW(mkn::kul::Exit);

 public:
  Settings(std::string const& s);

  const Settings* super() const { return sup.get(); }

  const mkn::kul::yaml::Validator validator() const;
  std::vector<std::string> const& remoteModules() const { return rms; }
  std::vector<std::string> const& remoteRepos() const { return rrs; }
  const mkn::kul::hash::map::S2S& properties() const { return ps; }

  static Settings& INSTANCE() KTHROW(mkn::kul::Exit);
  static bool SET(std::string const& s);
  static std::string RESOLVE(std::string const& s) KTHROW(SettingsException);
  static void POST_CONSTRUCT();
  static mkn::kul::cli::EnvVar PARSE_ENV_NODE(YAML::Node const&, Settings const&);
};
}  // namespace maiken
#endif /* _MAIKEN_SETTINGS_HPP_ */
