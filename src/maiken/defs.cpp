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

constexpr const char* maiken::Constants::STR_MODE;

constexpr const char* maiken::Constants::STR_ENV;
constexpr const char* maiken::Constants::STR_INC;
constexpr const char* maiken::Constants::STR_SRC;

constexpr const char* maiken::Constants::STR_TYPE;

constexpr const char* maiken::Constants::STR_BIN;
constexpr const char* maiken::Constants::STR_LIB;
constexpr const char* maiken::Constants::STR_GIT;
constexpr const char* maiken::Constants::STR_SVN;
constexpr const char* maiken::Constants::STR_SCM;
constexpr const char* maiken::Constants::STR_OUT;
constexpr const char* maiken::Constants::STR_NAME;
constexpr const char* maiken::Constants::STR_MASK;

constexpr const char* maiken::Constants::STR_VERSION;
constexpr const char* maiken::Constants::STR_PARENT;
constexpr const char* maiken::Constants::STR_SUPER;
constexpr const char* maiken::Constants::STR_PATH;
constexpr const char* maiken::Constants::STR_PACK;
constexpr const char* maiken::Constants::STR_BUILD;
constexpr const char* maiken::Constants::STR_FILE;
constexpr const char* maiken::Constants::STR_ARG;
constexpr const char* maiken::Constants::STR_INSTALL;
constexpr const char* maiken::Constants::STR_SELF;

constexpr const char* maiken::Constants::STR_VALUE;
constexpr const char* maiken::Constants::STR_MAIN;
constexpr const char* maiken::Constants::STR_TEST;
constexpr const char* maiken::Constants::STR_LANG;
constexpr const char* maiken::Constants::STR_LINK;
constexpr const char* maiken::Constants::STR_LOCAL;
constexpr const char* maiken::Constants::STR_REMOTE;
constexpr const char* maiken::Constants::STR_REPO;
constexpr const char* maiken::Constants::STR_MOD_REPO;
constexpr const char* maiken::Constants::STR_DEBUGGER;
constexpr const char* maiken::Constants::STR_DBG;
constexpr const char* maiken::Constants::STR_RUN;
constexpr const char* maiken::Constants::STR_INIT;

constexpr const char* maiken::Constants::STR_PROJECT;
constexpr const char* maiken::Constants::STR_ARCHIVER;
constexpr const char* maiken::Constants::STR_COMPILER;
constexpr const char* maiken::Constants::STR_COMPILE;
constexpr const char* maiken::Constants::STR_LINKER;

constexpr const char* maiken::Constants::STR_PROFILE;
constexpr const char* maiken::Constants::STR_PROFILES;
constexpr const char* maiken::Constants::STR_PROPERTY;
constexpr const char* maiken::Constants::STR_MOD;
constexpr const char* maiken::Constants::STR_MODS;
constexpr const char* maiken::Constants::STR_DEP;
constexpr const char* maiken::Constants::STR_DEPS;
constexpr const char* maiken::Constants::STR_QUIET;

constexpr const char* maiken::Constants::STR_IF_DEP;
constexpr const char* maiken::Constants::STR_IF_MOD;
constexpr const char* maiken::Constants::STR_IF_ARG;
constexpr const char* maiken::Constants::STR_IF_INC;
constexpr const char* maiken::Constants::STR_IF_LIB;
constexpr const char* maiken::Constants::STR_IF_LNK;
constexpr const char* maiken::Constants::STR_IF_SRC;

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
constexpr const char* maiken::Constants::STR_NODES;
#endif  //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_
