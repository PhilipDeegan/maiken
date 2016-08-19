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
#ifndef _MAIKEN_DEFS_LANG_HPP_
#define _MAIKEN_DEFS_LANG_HPP_

#define MKN_DEFS_CMD        "Commands:"
#define MKN_DEFS_BUILD      "   build    | Compile and link"
#define MKN_DEFS_CLEAN      "   clean    | Delete files from ./bin/$profile"
#define MKN_DEFS_COMP       "   compile  | Compile sources to ./bin/$profile"
#define MKN_DEFS_DBG        "   dbg      | Executes project profile binary with debugger"
#define MKN_DEFS_INIT       "   init     | Create minimal mkn.xml in ./"
#define MKN_DEFS_LINK       "   link     | Link object files to exe/lib"
#define MKN_DEFS_PACK       "   pack     | Copy binary files & library files in bin/$profile/pack"
#define MKN_DEFS_PROFS      "   profiles | Display profiles contained within ./mkn.xml"
#define MKN_DEFS_RUN        "   run      | Executes project profile binary linking dynamic libraries automatically"
#define MKN_DEFS_INC        "   inc      | Print include directories to std out"
#define MKN_DEFS_SRC        "   src      | Print found source files to std out [allows -d]."
#define MKN_DEFS_TRIM       "   trim     | Removes trailing whitespace from inc/src files under project directory"

#define MKN_DEFS_ARG        "Arguments:"
#define MKN_DEFS_ARGS       "   -a/--args $a          | Add $arg to compilation on build, passes to process on run"
#define MKN_DEFS_DEPS       "   -d/--dependencies $d  | Number of children to include or project/profile CSV, missing assumed infinite"
#define MKN_DEFS_DIRC       "   -C/--directory $d     | Execute on directory $d rather than current"
#define MKN_DEFS_EVSA       "   -E/--env $a           | CSV key=value environment variables override, format \"k1=v1,k2=v2\""
#define MKN_DEFS_HELP       "   -h/--help             | Print help to console"
#define MKN_DEFS_JARG       "   -j/--jargs            | File type specifc args as json like '{\"c\": \"-DC_ARG1\", \"cpp\": \"-DCXX_ARG1\"}'"
#define MKN_DEFS_LINKER     "   -l/--linker $t        | Adds $t to linking of root project profile"
#define MKN_DEFS_PROF       "   -p/--profile $p       | Activates profile $p"
#define MKN_DEFS_PROP       "   -P/--property $p      | CSV key=value project properties override, format \"k1=v1,k2=v2\""
#define MKN_DEFS_DRYR       "   -R/--dry-run          | Do not compile but print out process commands, do not run etc"
#define MKN_DEFS_STAT       "   -s/--scm-status       | Display SCM status of project directory, allows -d"
#define MKN_DEFS_THREDS     "   -t/--threads $n       | Consume $n threads while compiling source files where $n > 0, missing optimal resolution attempted."
#define MKN_DEFS_UPDATE     "   -u/--scm-update       | Check for updates per project and ask if to, allows -d"
#define MKN_DEFS_FUPDATE    "   -U/--scm-force-update | Force update project from SCM, allows -d"
#define MKN_DEFS_VERSON     "   -v/--version          | Displays the current maiken version number then exits, first checked command/argument"
#define MKN_DEFS_SETTNGS    "   -x/--settings $f      | Sets settings.xml in use to file $f, directory of $f missing $(HOME)/maiken/$f attempted"
#define MKN_DEFS_STATIC     "   -K/--static           | Links projects without mode as static"
#define MKN_DEFS_SHARED     "   -S/--shared           | Links projects without mode as shared"

#define MKN_DEFS_EXMPL      "Examples:"
#define MKN_DEFS_EXMPL1     "   mkn build -dtKUa             | Force update everything / Compile everything with optimal threads and, link everything statically"
#define MKN_DEFS_EXMPL2     "   mkn clean build -dtu         | Optionally update everything, clean everything, build everything with optimal threads"
#define MKN_DEFS_EXMPL3     "   mkn clean build -d 1 -t 2 -u | Update/Clean/Build project and immediate dependencies with two threads"
#define MKN_DEFS_EXMPL4     "   mkn -ds                      | Display '${scm} status' for everything"

#define MKN_PARENT          "Parent"
#define MKN_PROFILE         "Profile"
#define MKN_PROJECT         "Project"

#endif /* _MAIKEN_DEFS_LANG_HPP_ */
