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
#include "maiken/defs.hpp"

constexpr char const* maiken::Constants::STR_MODE;

constexpr char const* maiken::Constants::STR_ENV;
constexpr char const* maiken::Constants::STR_INC;
constexpr char const* maiken::Constants::STR_SRC;

constexpr char const* maiken::Constants::STR_TYPE;

constexpr char const* maiken::Constants::STR_BIN;
constexpr char const* maiken::Constants::STR_LIB;
constexpr char const* maiken::Constants::STR_GIT;
constexpr char const* maiken::Constants::STR_SVN;
constexpr char const* maiken::Constants::STR_SCM;
constexpr char const* maiken::Constants::STR_OUT;
constexpr char const* maiken::Constants::STR_NAME;
constexpr char const* maiken::Constants::STR_MASK;
constexpr char const* maiken::Constants::STR_WITH;

constexpr char const* maiken::Constants::STR_VERSION;
constexpr char const* maiken::Constants::STR_PARENT;
constexpr char const* maiken::Constants::STR_SUPER;
constexpr char const* maiken::Constants::STR_PATH;
constexpr char const* maiken::Constants::STR_PACK;
constexpr char const* maiken::Constants::STR_BUILD;
constexpr char const* maiken::Constants::STR_FILE;
constexpr char const* maiken::Constants::STR_ARG;
constexpr char const* maiken::Constants::STR_INSTALL;
constexpr char const* maiken::Constants::STR_SELF;

constexpr char const* maiken::Constants::STR_VALUE;
constexpr char const* maiken::Constants::STR_MAIN;
constexpr char const* maiken::Constants::STR_TEST;
constexpr char const* maiken::Constants::STR_LANG;
constexpr char const* maiken::Constants::STR_LINK;
constexpr char const* maiken::Constants::STR_LOCAL;
constexpr char const* maiken::Constants::STR_REMOTE;
constexpr char const* maiken::Constants::STR_REPO;
constexpr char const* maiken::Constants::STR_MOD_REPO;
constexpr char const* maiken::Constants::STR_DEBUGGER;
constexpr char const* maiken::Constants::STR_DBG;
constexpr char const* maiken::Constants::STR_RUN;
constexpr char const* maiken::Constants::STR_INIT;

constexpr char const* maiken::Constants::STR_PROJECT;
constexpr char const* maiken::Constants::STR_ARCHIVER;
constexpr char const* maiken::Constants::STR_COMPILER;
constexpr char const* maiken::Constants::STR_COMPILE;
constexpr char const* maiken::Constants::STR_LINKER;

constexpr char const* maiken::Constants::STR_PROFILE;
constexpr char const* maiken::Constants::STR_PROFILES;
constexpr char const* maiken::Constants::STR_PROPERTY;
constexpr char const* maiken::Constants::STR_MOD;
constexpr char const* maiken::Constants::STR_MODS;
constexpr char const* maiken::Constants::STR_DEP;
constexpr char const* maiken::Constants::STR_DEPS;
constexpr char const* maiken::Constants::STR_QUIET;

constexpr char const* maiken::Constants::STR_IF_DEP;
constexpr char const* maiken::Constants::STR_IF_MOD;
constexpr char const* maiken::Constants::STR_IF_ARG;
constexpr char const* maiken::Constants::STR_IF_INC;
constexpr char const* maiken::Constants::STR_IF_LIB;
constexpr char const* maiken::Constants::STR_IF_LNK;
constexpr char const* maiken::Constants::STR_IF_SRC;

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
constexpr char const* maiken::Constants::STR_NODES;
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_
