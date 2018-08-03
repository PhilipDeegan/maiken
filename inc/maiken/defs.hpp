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
#ifndef _MAIKEN_DEFS_HPP_
#define _MAIKEN_DEFS_HPP_

#include "kul/def.hpp"

#ifndef MKN_LANG
#include "maiken/lang/defs.en.hpp"
#else
#define __MAIKEN_INCLUDE_MACRO__(str) <maiken/lang/defs.##str.hpp>
#include __MAIKEN_INCLUDE_MACRO__(MKN_LANG)
#undef __MAIKEN_INCLUDE_MACRO__
#endif /*  MKN_LANG */

#ifndef _MKN_REP_NAME_DOT_
#define _MKN_REP_NAME_DOT_ 1
#endif /* _MKN_REP_NAME_DOT_ */

#ifndef _MKN_REP_VERS_DOT_
#define _MKN_REP_VERS_DOT_ 0
#endif /* _MKN_REP_VERS_DOT_ */

#ifndef _MKN_REMOTE_EXEC_
#define _MKN_REMOTE_EXEC_ 1
#endif /* _MKN_REMOTE_EXEC_ */

#ifndef _MKN_REMOTE_REPO_
#define _MKN_REMOTE_REPO_ "https://github.com/mkn/ https://gitlab.com/mkn/"
#endif /* _MKN_REMOTE_REPO_ */

#ifndef _MKN_REMOTE_MOD_
#define _MKN_REMOTE_MOD_                                                       \
  "https://github.com/mkn-mod/ https://gitlab.com/mkn-mod/"
#endif /* _MKN_REMOTE_MOD_ */

#ifndef _MKN_TIMESTAMPS_
#define _MKN_TIMESTAMPS_ 0
#endif /* _MKN_TIMESTAMPS_ */

#ifndef _MKN_VERSION_
#define _MKN_VERSION_ master
#endif /* _MKN_VERSION_ */

#ifndef _KUL_PROC_LOOP_NSLEEP_
#define _KUL_PROC_LOOP_NSLEEP_ 10000000
#endif /* _KUL_PROC_LOOP_NSLEEP_ */

namespace maiken {
class Constants {
public:
  static constexpr const char *STR_VERSION = "version";

  static constexpr const char *STR_MAIKEN = "maiken";

  static constexpr const char *STR_BIN = "bin";
  static constexpr const char *STR_DBG = "dbg";
  static constexpr const char *STR_OBJ = "obj";

  static constexpr const char *STR_BUILD = "build";
  static constexpr const char *STR_DEBUG = "debug";
  static constexpr const char *STR_DEBUGGER = "debugger";
  static constexpr const char *STR_CLEAN = "clean";
  static constexpr const char *STR_COMPILE = "compile";
  static constexpr const char *STR_HELP = "help";
  static constexpr const char *STR_INIT = "init";
  static constexpr const char *STR_INFO = "info";
  static constexpr const char *STR_LINK = "link";
  static constexpr const char *STR_PACK = "pack";
  static constexpr const char *STR_RUN = "run";
  static constexpr const char *STR_THREADS = "threads";
  static constexpr const char *STR_TRIM = "trim";
  static constexpr const char *STR_TREE = "tree";

  static constexpr const char *STR_SCM_COMMIT = "scm-commit";
  static constexpr const char *STR_SCM_STATUS = "scm-status";
  static constexpr const char *STR_SCM_UPDATE = "scm-update";
  static constexpr const char *STR_SCM_FUPDATE = "scm-force-update";

  static constexpr const char *STR_APPEND = "append";
  static constexpr const char *STR_PREPEND = "prepend";
  static constexpr const char *STR_REPLACE = "replace";

  static constexpr const char *STR_REPO = "repo";
  static constexpr const char *STR_MOD_REPO = "mod-repo";
  static constexpr const char *STR_GIT = "git";
  static constexpr const char *STR_SVN = "svn";

  static constexpr const char *STR_LANG = "lang";
  static constexpr const char *STR_HEADER = "header";
  static constexpr const char *STR_MAIN = "main";
  static constexpr const char *STR_TEST = "test";
  static constexpr const char *STR_INSTALL = "install";
  static constexpr const char *STR_GET = "get";
  static constexpr const char *STR_OUT = "out";
  static constexpr const char *STR_QUIET = "quiet";

  static constexpr const char *STR_PROJECT = "project";
  static constexpr const char *STR_LOCAL = "local";
  static constexpr const char *STR_REMOTE = "remote";
  static constexpr const char *STR_SCM = "scm";
  static constexpr const char *STR_SELF = "self";
  static constexpr const char *STR_PROFILE = "profile";
  static constexpr const char *STR_PROFILES = "profiles";

  static constexpr const char *STR_ENV = "env";
  static constexpr const char *STR_VARS = "vars";

  static constexpr const char *STR_MODE = "mode";
  static constexpr const char *STR_NONE = "none";
  static constexpr const char *STR_SHARED = "shared";
  static constexpr const char *STR_STATIC = "static";
  static constexpr const char *STR_DRY_RUN = "dry-run";

  static constexpr const char *STR_ARCHIVER = "archiver";
  static constexpr const char *STR_ASSEMBLER = "assembler";
  static constexpr const char *STR_COMPILER = "compiler";
  static constexpr const char *STR_LINKER = "linker";
  static constexpr const char *STR_ALINKER = "all-linker";

  static constexpr const char *STR_MASK = "mask";

  static constexpr const char *STR_PROPERTY = "property";
  static constexpr const char *STR_FILE = "file";
  static constexpr const char *STR_ARG = "arg";
  static constexpr const char *STR_ADD = "additional";
  static constexpr const char *STR_JARG = "jarg";
  static constexpr const char *STR_SRC = "src";
  static constexpr const char *STR_BINC = "binc";
  static constexpr const char *STR_BPATH = "bpath";
  static constexpr const char *STR_INC = "inc";
  static constexpr const char *STR_FINC = "finc";
  static constexpr const char *STR_FPATH = "flib";
  static constexpr const char *STR_EXT = "ext";
  static constexpr const char *STR_LIB = "lib";
  static constexpr const char *STR_DEP = "dep";
  static constexpr const char *STR_DEPS = "deps";
  static constexpr const char *STR_MOD = "mod";
  static constexpr const char *STR_MODS = "mods";
  static constexpr const char *STR_PATH = "path";
  static constexpr const char *STR_DEPENDENCY = "dependency";
  static constexpr const char *STR_MODULE = "module";
  static constexpr const char *STR_NAME = "name";
  static constexpr const char *STR_PARENT = "parent";
  static constexpr const char *STR_TYPE = "type";
  static constexpr const char *STR_VALUE = "value";
  static constexpr const char *STR_SUPER = "super";
  static constexpr const char *STR_OPT = "optimize";
  static constexpr const char *STR_WARN = "warning";
  static constexpr const char *STR_WITH = "with";
  static constexpr const char *STR_WITHOUT = "without";

  static constexpr const char *STR_RUN_ARG = "run-arg";

  static constexpr const char *STR_DIR = "directory";
  static constexpr const char *STR_SETTINGS = "settings";

  static constexpr const char *STR_IF_DEP = "if_dep";
  static constexpr const char *STR_IF_MOD = "if_mod";
  static constexpr const char *STR_IF_ARG = "if_arg";
  static constexpr const char *STR_IF_INC = "if_inc";
  static constexpr const char *STR_IF_SRC = "if_src";
  static constexpr const char *STR_IF_LIB = "if_lib";
  static constexpr const char *STR_IF_LNK = "if_link";

#if defined(_MKN_WITH_MKN_RAM_) && defined(_MKN_WITH_IO_CEREAL_)
  static constexpr const char *STR_NODES = "nodes";
#endif //_MKN_WITH_MKN_RAM_) && _MKN_WITH_IO_CEREAL_
};
} // namespace maiken
#endif /* _MAIKEN_DEFS_HPP_ */
