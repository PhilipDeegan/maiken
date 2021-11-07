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
#ifndef _MAIKEN_ENV_HPP_
#define _MAIKEN_ENV_HPP_

#include "maiken/app.hpp"
#include "maiken/property.hpp"


namespace maiken {

template <typename HasProperties>
mkn::kul::cli::EnvVar PARSE_ENV_NODE(YAML::Node const& n, HasProperties const& hasProperties, std::string hasProperties_id) {

  using namespace mkn::kul::cli;
  if (n.IsScalar()) {
    auto bits = mkn::kul::String::ESC_SPLIT(n.Scalar(), '=');

    if (bits.size() != 2)
      KEXIT(1, "env string is invalid, expects one '=' only, string ")
          << n.Scalar() << "\n in: " << hasProperties_id;

    auto replace = [](std::string const& n, std::string& in, std::string f) {
      auto pos = in.find(f);
      if (pos != std::string::npos)
        if (pos == 0 || (pos > 0 && in[pos - 1] != '\\'))
          mkn::kul::String::REPLACE(in, f, std::string(mkn::kul::env::GET(n.c_str())));
    };
    replace(bits[0], bits[1], "$" + bits[0]);
    replace(bits[0], bits[1], "${" + bits[0] + "}");

    return EnvVar(bits[0], Properties::RESOLVE(hasProperties, bits[1]), EnvVarMode::REPL);
  }

  EnvVarMode mode = EnvVarMode::PREP;
  if (n[Constants::STR_MODE]) {
    if (n[Constants::STR_MODE].Scalar().compare(Constants::STR_APPEND) == 0)
      mode = EnvVarMode::APPE;
    else if (n[Constants::STR_MODE].Scalar().compare(Constants::STR_PREPEND) == 0)
      mode = EnvVarMode::PREP;
    else if (n[Constants::STR_MODE].Scalar().compare(Constants::STR_REPLACE) == 0)
      mode = EnvVarMode::REPL;
  }
  return EnvVar(n[Constants::STR_NAME].Scalar(), Properties::RESOLVE(hasProperties, n[Constants::STR_VALUE].Scalar()) , mode);
}

}  // namespace maiken
#endif /* _MAIKEN_ENV_HPP_ */
