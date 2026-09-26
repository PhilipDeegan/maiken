# Build behaviour

## Language (HELP WANTED!)

English is the default language.
To activate another language run:

```sh
mkn clean build -a MKN_LANG=fr
```

## Timestamp logging

Upon successful compilation of a project, source file modification timestamps are recorded under `./bin/<profile>/.mkn/src_stamp`, which are checked on the next build to see if recompilation is required.

Similarly, the aggregate of all non-hidden file modification timestamps are recorded under `./bin/<profile>/.mkn/inc_stamp`, which are also checked on the next build to see if recompilation is required.

This can be enabled by compiling with the argument `-DMKN_TIMESTAMPS=1`

This is disabled for languages like C# when the source is not compiled.

The clean command deletes the timestamp files, and recompiles everything.

## Initialising scripts

After a missing dependency is retrieved for the first time, an initial setup script is run to put the repository in the desired state.
The type of file differs per OS, the rules are thus.

| OS      | Script |
|---------|--------|
| Windows | `./mkn.bat` |
| Linux   | `./mkn.nix.sh`, or if missing, `./mkn.sh` |
| BSD     | `./mkn.bsd.sh`, or if missing, `./mkn.sh` |

This can be disabled by compiling with the argument `-D_MKN_REMOTE_EXEC_=0`

## Default build command

If the first line of mkn.yaml starts with `#! `,
the subsequent string is used as the default command if mkn is executed with zero arguments or commands (`-C` is allowed).

```yaml
#! clean build -Kl -pthread

name: example
version: version
main: cpp.cpp
```

## Debug / optimization and warning flags

Options `-O` / `-g` / `-W` are provided to pass compiler and linker specific arguments for Optimizations, Debug symbols and warnings.

For each argument "9" is the highest value - and 0 is the lowest and is considered to be "disable"

- For optimizations - 9 will try to create programs optimized for the compiling machine.
- For debug - 9 will have the highest debugging - while 0 will be considered to disable. i.e. `-g 0` might have less debugging than without `-g` at all.
- For warnings - 8 is default when no value is given - 9 will include failure for any warnings when possible.
