# settings.yaml

The settings file is the default global configuration for projects.

## Schema

```yaml
super: $file                        # Optional, a settings file this one extends, e.g. "settings" for the default
                                    #  one, see "How settings files combine"

property:
    aaa: AAA
    bbb: ${aaa}.BBB
    mkn.link.rpathing: true           # Optional, default true. When building a SHAR
                                       # library or executable, add "-Wl,-rpath=..."
                                       # entries for its dependency libraries' paths.
                                       # Dependency libraries are always linked
                                       # ("-l"/"-L") regardless of this setting - it
                                       # only controls whether they're also rpathed.
    mkn.compile.visibility_hidden: false # Optional, default false. Compile with
                                       # "-fvisibility=hidden" (nvcc: through
                                       # -Xcompiler): symbols exported only where
                                       # annotated, as python modules want. Project
                                       # args after it may override it.

    mkn.flags.lto: ""                 # Optional. The compiler's default flags for the
    mkn.compile.flags.optimise: -O2   #  chosen -O/-g/-W levels come in named parts,
    mkn.link.flags.lto: ""            #  these replace one: mkn.flags.<part> for compile
                                       #  and link, mkn.compile.flags.<part> /
                                       #  mkn.link.flags.<part> for one side, over
                                       #  mkn.flags.<part>. An empty value drops the part,
                                       #  a name not among the defaults adds a new part.
                                       #  Which parts there are depends on the compiler
                                       #  and level: at least optimise, debug and warn
                                       #  where a level sets flags, some split further,
                                       #  e.g. gcc/clang -O 9 is optimise "-O3
                                       #  -funroll-loops", arch "-march=native
                                       #  -mtune=native" and lto "-flto=auto"/"-flto=thin"
                                       #  (linux) on both compile and link. A static
                                       #  library's archiver gets none of these, its
                                       #  file: archiver entry is used as given.

    mkn.bin.key: os=nix;...           # Optional, what prebuilt binaries (bin:) must match, defaults to
                                       #  this mkn's platform, "mkn -G mkn.bin.key" prints it and
                                       #  "mkn -vv" shows the default and a settings override. The
                                       #  default's machine parts (os, arch, libc) are detected, its
                                       #  toolchain parts (cxxrt, crt) are what releases use: a settings
                                       #  file for another toolchain (clang -stdlib=libc++, cl -MT)
                                       #  sets this, see inc/maiken/bin_key.hpp

    mkn.with.bin: 1                    # Optional, default 0. Every dependency not in bin: uses its
                                       #  version's prebuilt release when there is one: latest for
                                       #  the latest branch, the tag for a clone at that tag; other
                                       #  branches (master) build from source. A missing release is
                                       #  reported once, remembered in the clone's .mkn/prebuilt
                                       #  until its commit or mkn.bin.key changes. The environment
                                       #  variable MKN_WITH_BIN overrides it, -P mkn.with.bin=1 both

    mkn.env.automatic: true           # Optional, default true. Automatically prepend
                                       # dependency library directories onto PATH
                                       # (Windows) / LD_LIBRARY_PATH (nix) / also
                                       # DYLD_LIBRARY_PATH (macOS) before running or
                                       # testing a binary. Windows has no rpath
                                       # equivalent, so disabling this on Windows will
                                       # break running anything with non-default-search
                                       # shared dependencies.

local:
    repo: <directory>                   # Optional, missing assumed <settings location>/repo
    bin:  <directory>                   # Optional, successfully linked binaries are moved to <directory>, overrides install
    lib:  <directory>                   # Optional, successfully linked libraries are moved to <directory>, overrides install
    debugger: <debug command>           # Optional debug command, overrides defaut, overriden by env var MKN_DBG string
remote:
    repo: URL_ROOTA URL_ROOTB           # Optional, overrides switch MKN_REMOTE_REPO for incomplete SCM URL lookups

inc: <directory> <directory>            # Optional, add include directories compiling
path: <directory> <directory>           # Optional, add library path directories linking

env:                                    # Optional, useful for compiler specific settings
  - name: name
    mode: prepend/append/replace        # Optional default prepend
    value: value

compiler:
    mask:                               # To use a compiler binary with a name other than the default
        cl: $var_cl                     # masking is provided to alter the command line call
        gcc: $var_gcc                   # while using the same functionality as the mask key
                                        # e.g. gcc: gcc-armhf would allow the compiler tag to contain
                                        # gcc-armhf instead of just "gcc"

bin:                                    # Optional, dependencies used as prebuilt binaries instead of being
  parse.yaml: 0.8.0                     #  built: project name to release tag, or latest for the release
                                        #  GitHub marks Latest. Downloaded from the project's
                                        #  GitHub release (mkn.manifest.yml) into its clone's
                                        #  .mkn/prebuilt/<static|shared>, its build directory, which
                                        #  can be deleted to download again. Never compiled, linked or
                                        #  cleaned; a binary that can't be used is built from source with
                                        #  -q, otherwise mkn asks

file:                                   # Must include at least one item in list
  - type: c:cpp:cxx                     # Sources won't be compiled if the filetype is missing
    archiver: ${aaa}                    # Archiver is not used for C#
    compiler: ${aaa}
    linker:   ${aaa}
```

## Location

| OS      | Path |
|---------|------|
| Linux   | `~/.maiken/settings.yaml` |
| BSD/Mac | `~/.maiken/settings.yaml` |
| Windows | IF CYGWIN/MSYS `~/maiken/settings.yaml`<br>IF CMD `%HOMEDRIVE%/%HOMEPATH%/maiken/settings.yaml` |

The default file is created if missing.

A different file can be used with `-x $f`. Absolute paths are used as is, otherwise `$f` is looked
up in the current directory then the default settings directory above. `.yml`/`.yaml` suffixes are
tried if `$f` is not found.

## How settings files combine

A settings file can extend another with `super:`, and that one can extend another in turn. With
`-x`, the file given is the first of such a chain, which usually ends at the default settings
file (`super: settings`). Without `-x` the chain starts at the default file.

### Finding a file

`-x <name>` and `super: <name>` use an absolute path as is. Otherwise `<name>` is looked up in these
directories in order, trying `<name>`, `<name>.yml` then `<name>.yaml` in each:

| | `-x` | `super:` |
|---|---|---|
| 1 | the current directory | the current directory |
| 2 | | the directory of the file with the `super:` |
| 3 | the default settings directory, see Location | the default settings directory |

So `super: settings` is normally the default settings file, unless the current directory has a
`settings.yaml`. A file can't extend itself, and a cycle of `super:` files is an error.

### Combining entries

From the first file of the chain to the last:

| Entry | Across the chain |
|---|---|
| `file:` | per file type and key (`compiler`, `linker`, `archiver`) the first file setting it wins, so a file can set just one key for a type and take the rest from the files it extends |
| `property:` | a file's own properties win over those of the files it extends |
| `local:` `repo` / `mod-repo` | the first file that sets it |
| `remote:` | only the first file's, with the built-in defaults (`MKN_REMOTE_REPO` / `MKN_REMOTE_MOD`); a `remote:` further down the chain is not used |
| `inc:`, `path:` | every file's, the first file's first |
| `env:` | every file's, applied from the last file up, so the first file's `replace` is the one left and its `prepend` ends up in front |
| `compiler:` `mask:` | every file's |
| `find:` | every file's, per name the first file's directories first |
| `bin:` | per project, the first file naming it |

For example `res/mkn/bin_win.yml` in maiken's repository extends the default settings and sets only
`compiler: cl -MD` for C and C++, what the prebuilt binaries need; linker, archiver, includes and
library paths come from the default settings at the end of the chain.
