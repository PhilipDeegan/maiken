# settings.yaml

The settings file is the default global configuration for projects.

## Schema

```yaml
super: $directory                   # Inherit properties of project at directory, super (..) or sub directory advisable

property:
    aaa: AAA
    bbb: ${aaa}.BBB
    mkn.link.rpathing: true           # Optional, default true. When building a SHAR
                                       # library or executable, add "-Wl,-rpath=..."
                                       # entries for its dependency libraries' paths.
                                       # Dependency libraries are always linked
                                       # ("-l"/"-L") regardless of this setting - it
                                       # only controls whether they're also rpathed.

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
