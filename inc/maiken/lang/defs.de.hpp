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

#define MKN_DEFS_CMD        "Program commands:"
#define MKN_DEFS_BUILD      "\tbuild    | Compile le sources to ./bin/$profile and link"
#define MKN_DEFS_CLEAN      "\tclean    | Delete files from ./bin/$profile"
#define MKN_DEFS_COMP       "\tcompile  | Compile sources to ./bin/$profile"
#define MKN_DEFS_INIT       "\tinit     | Create minimal mkn.xml in ./"
#define MKN_DEFS_LINK       "\tlink     | Link object files to exe/lib"
#define MKN_DEFS_PROFS      "\tprofiles | Display profiles contained within ./mkn.xml"
#define MKN_DEFS_RUN        "\trun      | Executes project binary linking dynamic libraries automatically"

#define MKN_DEFS_ARG        "Program arguments:"
#define MKN_DEFS_ARGS       "\t--args -a $arg    | Add $arg to compilation of all files"
#define MKN_DEFS_DEPS       "\t--dependencies -d | $num | Include dependencies where num > 0, 1 includes immediate dependencies, missing assumed infinite"
#define MKN_DEFS_HELP       "\t--help -h         | Print help to console"
#define MKN_DEFS_PROF       "\t--profile -p $p   | Activates profile $p"
#define MKN_DEFS_STAT       "\t--scm-status -s   | Display SCM status of project directory, allows -d"
#define MKN_DEFS_THREDS     "\t--threads -t $n   | Consume $n threads while compiling source files where $n > 0, missing optimal resolution attempted."
#define MKN_DEFS_UPDATE     "\t--scm-update -u   | Update project from SCM, allows -d"
#define MKN_DEFS_VERSON     "\t--version -v      | Displays the current maiken version number then exits, first checked command/argument"
#define MKN_DEFS_SETTNGS    "\t--settings -x f   | Sets settings.xml in use to file $f, directory of $f missing $(HOME)/maiken/$f attempted"

#define MKN_DEFS_EXMPL      "Example uses:"
#define MKN_DEFS_EXMPL1     "\tmkn clean build -dtu           - Update everything, clean everything, build everything with optimal threads"
#define MKN_DEFS_EXMPL2     "\tmkn clean build -d 1 -t 2 -u   - Update/Clean/Build project and immediate dependencies with two threads"
#define MKN_DEFS_EXMPL3     "\tmkn -ds                        - Display '${scm} status' for everything"

#define MKN_PARENT          "Parent"
#define MKN_PROFILE         "Profile"
#define MKN_PROJECT         "Project"

#endif /* _MAIKEN_DEFS_LANG_HPP_ */
