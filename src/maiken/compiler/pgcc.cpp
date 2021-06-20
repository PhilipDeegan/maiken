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
#include "maiken.hpp"

maiken::cpp::PGCC_Compiler::PGCC_Compiler()  {
  m_optimise_c.insert({{0, ""},
                       {1, "-O1"},
                       {2, "-O2"},
                       {3, "-O3"},
                       {4, "-O3 "},
                       {5, "-O3 "},
                       {6, "-O3 "},
                       {7, "-O3 "},
                       {8, "-O3 "},
                       {9, "-O3 "}});
  m_debug_c.insert({{0, "-DNDEBUG"},
                    {1, "-g1"},
                    {2, "-g2"},
                    {3, "-g3"},
                    {4, "-g3"},
                    {5, "-g3"},
                    {6, "-g3"},
                    {7, "-g3"},
                    {8, "-g3"},
                    {9, "-g3"}});
  m_optimise_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_optimise_l_lib.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_debug_l_bin.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_debug_l_lib.insert(
      {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
  m_warn_c.insert({{0, "-w"},
                   {1, "-Wall"},
                   {2, "-Wall"},
                   {3, "-Wall"},
                   {4, "-Wall"},
                   {5, "-Wall"},
                   {6, "-Wall"},
                   {7, "-Wall"},
                   {8, "-Wall -Wextra"},
                   {9, "-Wall -Wextra -pedantic -Werror"}});
}
