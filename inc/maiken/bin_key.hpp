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
#ifndef _MAIKEN_BIN_KEY_HPP_
#define _MAIKEN_BIN_KEY_HPP_

// The default mkn.bin.key: what a prebuilt binary must match on this machine. Preprocessor only,
//  no other maiken or mkn.kul code.
//
// Machine parts are detected from how this mkn was compiled, it runs where it builds:
//   os    win, bsd (macOS), nix (Linux), freebsd, openbsd, netbsd, else unknown
//   arch  x86_64, arm64, x86, arm, riscv64, ppc64le, s390x, else unknown
//   libc  Linux only: manylinux_2_28 for glibc 2.28 or newer (binaries built against 2.28 run on
//         any later glibc), manylinux_<major>_<minor> for an older glibc, uclibc, bionic, else
//         musl (it has no macro of its own)
// Toolchain parts are what releases are published with on each os, mkn's own compiler says
//  nothing about the one a project uses:
//   cxxrt msvc (win), libc++ (bsd), libstdc++ (nix)
//   crt   md (win), macos 13.0 (bsd)
// Only what is positively detected is claimed: anything else gets a value no release publishes,
//  so it is built from source. A toolchain that differs (clang -stdlib=libc++ on Linux, cl -MT)
//  or a machine detected wrongly sets settings property: mkn.bin.key instead.

#include <climits>  // a libc header: glibc, uClibc, musl and bionic define their macros with it

#define MKN_BIN_KEY_STR_(x) #x
#define MKN_BIN_KEY_STR(x) MKN_BIN_KEY_STR_(x)

#if defined(_WIN32)
#define MKN_BIN_KEY_OS "win"
#elif defined(__APPLE__)
#define MKN_BIN_KEY_OS "bsd"
#elif defined(__linux__)
#define MKN_BIN_KEY_OS "nix"
#elif defined(__FreeBSD__)
#define MKN_BIN_KEY_OS "freebsd"
#elif defined(__OpenBSD__)
#define MKN_BIN_KEY_OS "openbsd"
#elif defined(__NetBSD__)
#define MKN_BIN_KEY_OS "netbsd"
#else
#define MKN_BIN_KEY_OS "unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define MKN_BIN_KEY_ARCH "x86_64"
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
#define MKN_BIN_KEY_ARCH "arm64"
#elif defined(__i386__) || defined(_M_IX86)
#define MKN_BIN_KEY_ARCH "x86"
#elif defined(__arm__) || defined(_M_ARM)
#define MKN_BIN_KEY_ARCH "arm"
#elif defined(__riscv) && __riscv_xlen == 64
#define MKN_BIN_KEY_ARCH "riscv64"
#elif defined(__powerpc64__) && defined(__LITTLE_ENDIAN__)
#define MKN_BIN_KEY_ARCH "ppc64le"
#elif defined(__s390x__)
#define MKN_BIN_KEY_ARCH "s390x"
#else
#define MKN_BIN_KEY_ARCH "unknown"
#endif

#if defined(__UCLIBC__)  // before glibc, uClibc defines __GLIBC__ too
#define MKN_BIN_KEY_LIBC "uclibc"
#elif defined(__BIONIC__)
#define MKN_BIN_KEY_LIBC "bionic"
#elif defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 28))
#define MKN_BIN_KEY_LIBC "manylinux_2_28"
#elif defined(__GLIBC__)
#define MKN_BIN_KEY_LIBC \
  "manylinux_" MKN_BIN_KEY_STR(__GLIBC__) "_" MKN_BIN_KEY_STR(__GLIBC_MINOR__)
#else
#define MKN_BIN_KEY_LIBC "musl"
#endif

#if defined(_WIN32)
#define MKN_BIN_KEY "os=" MKN_BIN_KEY_OS ";arch=" MKN_BIN_KEY_ARCH ";cxxrt=msvc;crt=md"
#elif defined(__APPLE__)
#define MKN_BIN_KEY "os=" MKN_BIN_KEY_OS ";arch=" MKN_BIN_KEY_ARCH ";cxxrt=libc++;macos=13.0"
#elif defined(__linux__)
#define MKN_BIN_KEY \
  "os=" MKN_BIN_KEY_OS ";arch=" MKN_BIN_KEY_ARCH ";cxxrt=libstdc++;libc=" MKN_BIN_KEY_LIBC
#else
#define MKN_BIN_KEY "os=" MKN_BIN_KEY_OS ";arch=" MKN_BIN_KEY_ARCH
#endif

#endif /* _MAIKEN_BIN_KEY_HPP_ */
