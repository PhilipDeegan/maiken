# Platforms

Reference of platform/compiler combinations maiken has been built on, with the extra compiler
arguments each needs. maiken requires C++20, so pass `-std=c++20` (gcc/clang) or `-std:c++20`
(MSVC) alongside these.

| Arch     | OS      | Compiler | Bits | Arguments |
|----------|---------|----------|------|-----------|
| x86_64   | debian  | gcc      | 32   | `-Wall` |
| x86_64   | debian  | gcc      | 64   | `-Wall` |
| x86_64   | netbsd  | gcc      | 32   | `-Wall` |
| x86_64   | netbsd  | gcc      | 64   | `-Wall` |
| x86_64   | freebsd | gcc      | 32   | `-D_GLIBCXX_USE_C99 -Wall` |
| x86_64   | freebsd | gcc      | 64   | `-D_GLIBCXX_USE_C99 -Wall` |
| armv7_a  | debian  | gcc      | 32   | `-Wall` |
| x86_64   | windows | cl       | 32   | `-EHsc` |
| x86_64   | windows | cl       | 64   | `-EHsc` (untested) |
| arm      | windows | cl       | 32   | `-EHsc` |

For building maiken on each platform see [Building maiken](man/build.md).
