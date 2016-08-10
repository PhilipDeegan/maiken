/**
Copyright (c) 2013, Philip Deegan.
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
    #define _MKN_REMOTE_REPO_ "https://github.com/mkn/"
#endif /* _MKN_REMOTE_REPO_ */

#ifndef _MKN_TIMESTAMPS_
    #define _MKN_TIMESTAMPS_ 0
#endif /* _MKN_TIMESTAMPS_ */

#ifndef _MKN_VERSION_
    #define _MKN_VERSION_ master
#endif /* _MKN_VERSION_ */

namespace maiken{
class Constants{
    public:
        static constexpr const char* VERSION        = "version";

        static constexpr const char* MAIKEN         = "maiken";

        static constexpr const char* BIN            = "bin";
        static constexpr const char* DBG            = "dbg";
        static constexpr const char* OBJ            = "obj";

        static constexpr const char* BUILD          = "build";
        static constexpr const char* DEBUG          = "debug";
        static constexpr const char* DEBUGGER       = "debugger";
        static constexpr const char* CLEAN          = "clean";
        static constexpr const char* COMPILE        = "compile";
        static constexpr const char* HELP           = "help";
        static constexpr const char* INIT           = "init";
        static constexpr const char* INFO           = "info";
        static constexpr const char* LINK           = "link";
        static constexpr const char* RUN            = "run";
        static constexpr const char* THREADS        = "threads";
        static constexpr const char* TRIM           = "trim";

        static constexpr const char* SCM_COMMIT     = "scm-commit";
        static constexpr const char* SCM_STATUS     = "scm-status";
        static constexpr const char* SCM_UPDATE     = "scm-update";
        static constexpr const char* SCM_FUPDATE    = "scm-force-update";

        static constexpr const char* APPEND         = "append";
        static constexpr const char* PREPEND        = "prepend";
        static constexpr const char* REPLACE        = "replace";

        static constexpr const char* REPO           = "repo";
        static constexpr const char* GIT            = "git";
        static constexpr const char* SVN            = "svn";

        static constexpr const char* LANG           = "lang";
        static constexpr const char* HEADER         = "header";
        static constexpr const char* MAIN           = "main";
        static constexpr const char* INSTALL        = "install";

        static constexpr const char* PROJECT        = "project";
        static constexpr const char* LOCAL          = "local";
        static constexpr const char* REMOTE         = "remote";
        static constexpr const char* SCM            = "scm";
        static constexpr const char* SELF           = "self";
        static constexpr const char* PROFILE        = "profile";
        static constexpr const char* PROFILES       = "profiles";

        static constexpr const char* ENV            = "env";
        static constexpr const char* LIB            = "lib";
        static constexpr const char* VARS           = "vars";

        static constexpr const char* MODE           = "mode";
        static constexpr const char* NONE           = "none";
        static constexpr const char* SHARED         = "shared";
        static constexpr const char* STATIC         = "static";
        static constexpr const char* DRY_RUN        = "dry-run";

        static constexpr const char* ARCHIVER       = "archiver";
        static constexpr const char* ASSEMBLER      = "assembler";
        static constexpr const char* COMPILER       = "compiler";
        static constexpr const char* LINKER         = "linker";

        static constexpr const char* PROPERTY       = "property";
        static constexpr const char* FILE           = "file";
        static constexpr const char* ARG            = "arg";
        static constexpr const char* JARG           = "jarg";
        static constexpr const char* MKN_SRC        = "src";
        static constexpr const char* MKN_INC        = "inc";
        static constexpr const char* MKN_LIB        = "lib";
        static constexpr const char* MKN_DEP        = "dep";
        static constexpr const char* MKN_DEPS       = "deps";
        static constexpr const char* PATH           = "path";
        static constexpr const char* DEPENDENCY     = "dependency";
        static constexpr const char* NAME           = "name";
        static constexpr const char* PARENT         = "parent";
        static constexpr const char* TYPE           = "type";
        static constexpr const char* VALUE          = "value";
        static constexpr const char* SUPER          = "super";

        static constexpr const char* DIRECTORY      = "directory";
        static constexpr const char* SETTINGS       = "settings";

        static constexpr const char* IF_ARG         = "if_arg";
        static constexpr const char* IF_INC         = "if_inc";
        static constexpr const char* IF_SRC         = "if_src";
        static constexpr const char* IF_LIB         = "if_lib";
};
}
#endif /* _MAIKEN_DEFS_HPP_ */
