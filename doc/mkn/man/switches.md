# Switches

Compile-time switches for building maiken.

## `MKN_REP_NAME_DOT`

Type: bool, default: `1`

Split project name by period. If true

```yaml
name: pro.ject
version: version
```

will expect folder `${LOCAL_REPO}/pro/ject/version`, otherwise `${LOCAL_REPO}/pro.ject/version`

## `MKN_REP_VERS_DOT`

Type: bool, default: `0`

Split project version by period. If true

```yaml
name: project
version: ver.si.on
```

will expect folder `${LOCAL_REPO}/project/ver/si/on`, otherwise `${LOCAL_REPO}/project/ver.si.on`

## `MKN_TIMESTAMPS`

Type: bool, default: `0`

Logs timestamps of source/includes to skip files with no changes. See [timestamp logging](behaviour.md#timestamp-logging).

## `MKN_REMOTE_EXEC`

Type: bool, default: `1`

Execute mkn.(bat/sh etc) in directory of missing dependencies when retrieved from SCM. See [initialising scripts](behaviour.md#initialising-scripts).

## `MKN_REMOTE_REPO`

Type: string, default: `"http://github.com/mkn/"`

Space separated list of URLs to use as roots for non-complete SCM repositories.

```yaml
scm: http://github.com/mkn/mkn.kul.git  # ignored
scm: git@github.com:mkn/mkn.kul.git     # ignored
scm: mkn.kul.git                        # becomes
                                        #   for(const std::string& s: split(MKN_REMOTE_REPO, " ")
                                        #     attempt(s + "mkn.kul.git")
                                        #   if all attempts fail, throw
```

## `MKN_DISABLE_SCM`

Type: flag, default: `0`

Disables all SCM

## `MKN_DISABLE_SVN`

Type: flag, default: `0`

Disables SVN if it was supported

## `MKN_DISABLE_GIT`

Type: flag, default: `0`

Disables git

## `MKN_DISABLE_RUN_LIB_PATH_HANDLING`

Type: flag, default: `0`

| OS  | Effect |
|-----|--------|
| nix | disables `LD_LIBRARY_PATH` modifications during "run/dbg" commands |
| bsd | if osx: disables `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH` modifications during "run/dbg" commands<br>else: disables `LD_LIBRARY_PATH` modifications during "run/dbg" commands |
| win | disables `PATH` modifications during "run/dbg" commands |

## `MKN_GIT_WITH_RAM_DEFAULT_CO_ACTION`

Type: uint, default: `0`

Default action deduction for retrieving dependencies if built with mkn.ram

| Value | Action |
|-------|--------|
| `0` | get default branch |
| `1` | if has release, get, else if has tag, get, else 0 |

# With options

## `io.cereal`

Enables serialization of some classes - requirement for distributed compilation

## `mkn.ram[https]`

Enables calls to github.com API to deduce most recent release or branch if no
dependency version is set
