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
#ifndef MAIKEN_DEFS_HPP
#define MAIKEN_DEFS_HPP

#include "mkn/kul/env.hpp"
#include "mkn/kul/defs.hpp"

#include "mkn/kul/except.hpp"

#ifndef MKN_LANG
#include "maiken/lang/defs.en.hpp"
#else
#define MAIKEN_INCLUDE_MACRO(str) <maiken/lang/defs.str.hpp>
#include MAIKEN_INCLUDE_MACRO(MKN_LANG)
#undef MAIKEN_INCLUDE_MACRO
#endif /*  MKN_LANG */

using namespace maiken::lang::defs;

#ifndef MKN_REP_NAME_DOT
#define MKN_REP_NAME_DOT 1
#endif /* MKN_REP_NAME_DOT */

#ifndef MKN_REP_VERS_DOT
#define MKN_REP_VERS_DOT 0
#endif /* MKN_REP_VERS_DOT */

#ifndef MKN_REMOTE_EXEC
#define MKN_REMOTE_EXEC 1
#endif /* MKN_REMOTE_EXEC */

#ifndef MKN_REMOTE_REPO
#define MKN_REMOTE_REPO "https://u:p@github.com/mkn/ https://u:p@gitlab.com/mkn/"
#endif /* MKN_REMOTE_REPO */

#ifndef MKN_REMOTE_MOD
#define MKN_REMOTE_MOD "https://u:p@github.com/mkn-mod/ https://u:p@gitlab.com/mkn-mod/"
#endif /* MKN_REMOTE_MOD */

#ifndef MKN_TIMESTAMPS
#define MKN_TIMESTAMPS 0
#endif /* MKN_TIMESTAMPS */

#ifndef MKN_VERSION
#define MKN_VERSION master
#endif /* MKN_VERSION */

#ifndef MKN_GIT_WITH_RAM_DEFAULT_CO_ACTION
#define MKN_GIT_WITH_RAM_DEFAULT_CO_ACTION 0
#endif /* MKN_VERSION */

#ifndef MKN_KUL_PROC_LOOP_NSLEEP
#define MKN_KUL_PROC_LOOP_NSLEEP 1000000
#endif /* MKN_KUL_PROC_LOOP_NSLEEP */

namespace maiken {

auto inline defaultSCMBranchName() { return mkn::kul::env::GET("MKN_DEFAULT_BRANCH", "master"); }

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class Constants {
 public:
  static constexpr auto STR_MAIKEN = "maiken";
  static constexpr auto STR_VERSION = "version";

  static constexpr auto STR_BIN = "bin", STR_OBJ = "obj";

  static constexpr auto STR_BUILD = "build", STR_CLEAN = "clean";
  static constexpr auto STR_RUN = "run", STR_DBG = "dbg";
  static constexpr auto STR_DEBUG = "debug";
  static constexpr auto STR_DEBUGGER = "debugger";
  static constexpr auto STR_COMPILE = "compile";
  static constexpr auto STR_HELP = "help", STR_INIT = "init", STR_INFO = "info";
  static constexpr auto STR_LINK = "link";
  static constexpr auto STR_PACK = "pack";
  static constexpr auto STR_THREADS = "threads";
  static constexpr auto STR_TREE = "tree";

  static constexpr auto STR_SCM_COMMIT = "scm-commit";
  static constexpr auto STR_SCM_STATUS = "scm-status";
  static constexpr auto STR_SCM_UPDATE = "scm-update";
  static constexpr auto STR_SCM_FUPDATE = "scm-force-update";

  static constexpr auto STR_APPEND = "append";
  static constexpr auto STR_PREPEND = "prepend";
  static constexpr auto STR_REPLACE = "replace";

  static constexpr auto STR_REPO = "repo";
  static constexpr auto STR_MOD_REPO = "mod-repo";
  static constexpr auto STR_GIT = "git";
  static constexpr auto STR_SVN = "svn";

  static constexpr auto STR_LANG = "lang";
  static constexpr auto STR_HEADER = "header";
  static constexpr auto STR_MAIN = "main";
  static constexpr auto STR_TEST = "test";
  static constexpr auto STR_INSTALL = "install";
  static constexpr auto STR_GET = "get";
  static constexpr auto STR_OUT = "out";
  static constexpr auto STR_QUIET = "quiet";

  static constexpr auto STR_PROJECT = "project";
  static constexpr auto STR_LOCAL = "local";
  static constexpr auto STR_REMOTE = "remote";
  static constexpr auto STR_SCM = "scm";
  static constexpr auto STR_SELF = "self";
  static constexpr auto STR_SUB = "sub";
  static constexpr auto STR_PROFILE = "profile";
  static constexpr auto STR_PROFILES = "profiles";

  static constexpr auto STR_ENV = "env";
  static constexpr auto STR_VARS = "vars";

  static constexpr auto STR_MODE = "mode";
  static constexpr auto STR_NONE = "none";
  static constexpr auto STR_SHARED = "shared";
  static constexpr auto STR_STATIC = "static";
  static constexpr auto STR_DRY_RUN = "dry-run";

  static constexpr auto STR_ARCHIVER = "archiver";
  static constexpr auto STR_ASSEMBLER = "assembler";
  static constexpr auto STR_COMPILER = "compiler";
  static constexpr auto STR_LINKER = "linker";
  static constexpr auto STR_ALINKER = "all-linker";

  static constexpr auto STR_MASK = "mask";

  static constexpr auto STR_PROPERTY = "property";
  static constexpr auto STR_FILE = "file";
  static constexpr auto STR_ARG = "arg";
  static constexpr auto STR_BINC = "binc";
  static constexpr auto STR_BPATH = "bpath";
  static constexpr auto STR_ADD = "additional";
  static constexpr auto STR_DUMP = "dump";
  static constexpr auto STR_JARG = "jarg";
  static constexpr auto STR_INC = "inc";
  static constexpr auto STR_FINC = "finc";
  static constexpr auto STR_FORCE = "force";
  static constexpr auto STR_FPATH = "flib";
  static constexpr auto STR_LIB = "lib";
  static constexpr auto STR_DEP = "dep";
  static constexpr auto STR_DEPS = "deps";
  static constexpr auto STR_MOD = "mod";
  static constexpr auto STR_MODS = "mods";
  static constexpr auto STR_PATH = "path";
  static constexpr auto STR_DEPENDENCY = "dependency";
  static constexpr auto STR_MODULE = "module";
  static constexpr auto STR_NAME = "name";
  static constexpr auto STR_PARENT = "parent";
  static constexpr auto STR_SRC = "src";
  static constexpr auto STR_TYPE = "type";
  static constexpr auto STR_VALUE = "value";
  static constexpr auto STR_SUPER = "super";
  static constexpr auto STR_OPT = "optimize";
  static constexpr auto STR_WARN = "warning";
  static constexpr auto STR_WITH = "with";
  static constexpr auto STR_WITHOUT = "without";

  static constexpr auto STR_RUN_ARG = "run-arg";

  static constexpr auto STR_DIR = "directory";
  static constexpr auto STR_SETTINGS = "settings";

  static constexpr auto STR_IF_DEP = "if_dep";
  static constexpr auto STR_IF_MOD = "if_mod";
  static constexpr auto STR_IF_ARG = "if_arg";
  static constexpr auto STR_IF_INC = "if_inc";
  static constexpr auto STR_IF_SRC = "if_src";
  static constexpr auto STR_IF_LIB = "if_lib";
  static constexpr auto STR_IF_LNK = "if_link";

#if defined(MKN_WITH_MKN_RAM) && defined(MKN_WITH_IO_CEREAL)
  static constexpr auto STR_NODES = "nodes";
#endif  // MKN_WITH_MKN_RAM) && MKN_WITH_IO_CEREAL
};
}  // namespace maiken
#endif /* MAIKEN_DEFS_HPP */
