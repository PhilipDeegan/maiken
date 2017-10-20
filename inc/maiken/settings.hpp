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

#include "kul/log.hpp"
#include "kul/os.hpp"
#include "kul/yaml.hpp"

#include "maiken/defs.hpp"

namespace maiken {

class SettingsException : public kul::Exception
{
public:
  SettingsException(const char* f, const uint16_t& l, const std::string& s)
    : kul::Exception(f, l, s)
  {}
};

class Settings
  : public kul::yaml::File
  , public Constants
{
private:
  std::vector<std::string> rrs, rms;
  std::unique_ptr<Settings> sup;
  kul::hash::map::S2S ps;

  void resolveProperties() KTHROW(SettingsException);

  static std::unique_ptr<Settings> instance;
  static void write(const kul::File& f) KTHROW(kul::Exit);

public:
  Settings(const std::string& s);

  const Settings* super() const { return sup.get(); }

  const kul::yaml::Validator validator() const;
  const std::vector<std::string>& remoteModules() const { return rms; }
  const std::vector<std::string>& remoteRepos() const { return rrs; }
  const kul::hash::map::S2S& properties() const { return ps; }

  static Settings& INSTANCE() KTHROW(kul::Exit);
  static bool SET(const std::string& s);
  static std::string RESOLVE(const std::string& s) KTHROW(SettingsException);
};

} // namespace maiken
#endif /* _MAIKEN_SETTINGS_HPP_ */
