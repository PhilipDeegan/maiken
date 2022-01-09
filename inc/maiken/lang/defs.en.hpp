/**
Copyright (c) 2022, Philip Deegan.
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

#include "maiken/lang/err/en.hpp"

#define MKN_DEFS_CMD "Commands:"
#define MKN_DEFS_BUILD "   build     | Compile and link all active projects"
#define MKN_DEFS_CLEAN "   clean     | Delete files from ./bin/$profile"
#define MKN_DEFS_COMP "   compile   | Compile sources to ./bin/$profile"
#define MKN_DEFS_DBG "   dbg       | Executes project profile binary with debugger"
#define MKN_DEFS_INIT "   init      | Create minimal mkn.yaml in ./"
#define MKN_DEFS_LINK "   link      | Link object files to exe/lib"
#define MKN_DEFS_PACK "   pack      | Copy binary files & library files into bin/$profile/pack"
#define MKN_DEFS_PROFS "   profiles  | Display profiles contained within ./mkn.yaml"
#define MKN_DEFS_RUN                                                          \
  "   run       | Executes project profile binary linking dynamic libraries " \
  "automatically"
#define MKN_DEFS_INC "   inc       | Print include directories to std out"
#define MKN_DEFS_SRC "   src       | Print found source files to std out [allows -d]."

#define MKN_DEFS_TREE "   tree      | Display dependency tree"

#define MKN_DEFS_ARG "Arguments:"
#define MKN_DEFS_ARGS                                                        \
  "   -a/--args $a           | Add $arg to compilation on build, passed to " \
  "run if no build/compile processing"
#define MKN_DEFS_ADD                                               \
  "   -A --add $csv          | CSV list of additional sources to " \
  "compile/link, disable recursive finding with \"<dir>\\,0\""
#define MKN_DEFS_BINC                                                       \
  "   -b/--binc $s           | Add include directories to back of compile " \
  "command, seperated by standard system PATH environment win=; others=:"

#define MKN_DEFS_BPATH                                                        \
  "   -B/--bpath $s          | Add library search directory to back of link " \
  "command, seperated by standard system PATH environment win=; others=:"
#define MKN_DEFS_DIRC "   -C/--directory $d      | Execute on directory $d rather than current"
#define MKN_DEFS_DEPS                                             \
  "   -d/--dependencies [$d] | Number of children to include or " \
  "project/profile CSV, missing assumed infinite"

#define MKN_DEFS_DUMP "   -D/--dump              | Write command logs to ./.mkn/logs/$PROFILE"

#define MKN_DEFS_EVSA                                                          \
  "   -E/--env $a            | CSV key=value environment variables override, " \
  "format \"k1=v1,k2=v2\""
#define MKN_DEFS_GET                                                           \
  "   -G/--get $k            | Returns string value for property K in either " \
  "local yaml or settings.yaml, failure is no string 0 exit code"
#define MKN_DEFS_HELP "   -h/--help              | Print help to console"
#define MKN_DEFS_FINC                                                        \
  "   -f/--finc $s           | Add include directories to front of compile " \
  "command, seperated by standard system PATH environment win=; others=:"
#define MKN_DEFS_FPATH                                                         \
  "   -F/--fpath $s          | Add library search directory to front of link " \
  "command, seperated by standard system PATH environment win=; others=:"
#define MKN_DEFS_JARG                                                         \
  "   -j/--jargs             | File type specifc args as json like '{\"c\": " \
  "\"-DC_ARG1\", \"cpp\": \"-DCXX_ARG1\"}'"
#define MKN_DEFS_LINKER "   -l/--linker $t         | Adds $t to linking of root project profile"
#define MKN_DEFS_ALINKR                                                       \
  "   -L/--all-linker $t     | Adds $t to linking of all projects with link " \
  "operations"
#define MKN_DEFS_MOD \
  "   -m/--mod [$csv]        | Activate modules for selected phase (init/compile/link/pack) "
#define MKN_DEFS_MAIN                                                          \
  "   -M/--main $m           | Sets main linker file to $m, supercedes /main " \
  "tag"
#define MKN_DEFS_OUT                                                    \
  "   -o/--out $o            | Sets output binary/library to name $o, " \
  "supercedes /out tag"
#define MKN_DEFS_PROF "   -p/--profile $p        | Activates profile $p"
#define MKN_DEFS_PROP                                                       \
  "   -P/--property $p       | CSV key=value project properties override, " \
  "format \"k1=v1,k2=v2\""
#define MKN_DEFS_RUN_ARGS                                                  \
  "   -r/--run-args $a       | Passes $a to running binary as arguments, " \
  "suppercedes -a"
#define MKN_DEFS_DRYR                                                 \
  "   -R/--dry-run           | Do not compile but print out process " \
  "commands, do not run etc"
#define MKN_DEFS_STAT                                                     \
  "   -s/--scm-status        | Display SCM status of project directory, " \
  "allows -d"
#define MKN_DEFS_THREDS                                                    \
  "   -t/--threads [$n]      | Consume $n threads while compiling source " \
  "files where $n > 0, n missing optimal resolution attempted."
#define MKN_DEFS_UPDATE "   -u/--scm-update        | Check for updates per project and ask if to"
#define MKN_DEFS_FUPDATE "   -U/--scm-force-update  | Force update project from SCM"
#define MKN_DEFS_VERSON                                                     \
  "   -v/--version           | Displays the current maiken version number " \
  "then exits, first checked command/argument"
#define MKN_DEFS_SETTNGS                                               \
  "   -x/--settings $f       | Sets settings.yaml in use to file $f, " \
  "directory of $f missing $(HOME)/maiken/$f attempted"
#define MKN_DEFS_STATIC "   -K/--static            | Links projects without mode as static"
#define MKN_DEFS_SHARED "   -S/--shared            | Links projects without mode as shared"
#define MKN_DEFS_DEBUG "   -g/--debug [0-9]       | Add compiler/linker flags to debug"
#define MKN_DEFS_OPTIM "   -O/--optimize [0-9]    | Add compiler/linker flags to optimize"
#define MKN_DEFS_WARN "   -W/--warn [0-9]        | Add compiler flags for warnings"
#define MKN_DEFS_WITH "   -w/--with $CSV         | Add profile or dependency from command line"
#define MKN_DEFS_WITHOUT "   -T/--without $CSV      | Remove dependencies from build"

#define MKN_DEFS_EXMPL "Examples:"
#define MKN_DEFS_EXMPL1                                                  \
  "   mkn build -dtKUa             | Force update everything / Compile " \
  "everything with optimal threads and, link everything statically"
#define MKN_DEFS_EXMPL2                                                    \
  "   mkn clean build -dtu         | Optionally update everything, clean " \
  "everything, build everything with optimal threads"
#define MKN_DEFS_EXMPL3                                               \
  "   mkn clean build -d 1 -t 2 -u | Update/Clean/Build project and " \
  "immediate dependencies with two threads"
#define MKN_DEFS_EXMPL4 "   mkn -ds                      | Display '${scm} status' for everything"

#define MKN_PARENT "Parent"
#define MKN_PROFILE "Profile"
#define MKN_PROJECT "Project"
#define MKN_PROJECT_NOT_FOUND "Project not found, attempting automatic resolution: "

#endif /* _MAIKEN_DEFS_LANG_HPP_ */
