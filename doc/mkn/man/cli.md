# Command line

## Commands

| Command     | Description |
|-------------|-------------|
| `build`     | Compiles all supported files to the profile binary (`./bin/profile`) directory, and creates library/executable depending |
| `clean`     | deletes all files from the profile directory recursively. |
| `compile`   | Compiles all supported file to the profile binary directory |
| `link`      | Creates a library/executable from found compiled files |
| `profiles`  | displays `$(PWD)/mkn.yaml` profiles to std::out - then exits (`--version` takes precedence) |
| `init`      | Create a minimal project file `${PWD}/mkn.yaml`, exits, error if one exists |
| `inc`       | Print include directories to std out |
| `src`       | Print found source files to std out [allows `-d`]. |
| `pack`      | Copy binary files & library files into `bin/$profile/pack` |
| `run`       | Starts the application automatically linking libraries for dependencies on dynamic libraries, supports `-a` and `-p`/`-D` |
| `dbg`       | Same as run but uses debugger - see [debugging](debugging.md) |
| `tree [$p]` | Prints dependency tree for base profile (or profile `$p`) |
| `test`      | Builds and runs each file matched by the profile `test` tag, see [mkn.yaml](yaml.md) |
| `info`      | Displays the local repository, compile threads and the archiver/compiler/linker found per file type |

## Arguments

| Argument | Description |
|----------|-------------|
| `-a --args $arg`        | Adds `$arg` to the compile command if compile/build, passes arguments to application if run |
| `-A --add $arg`         | CSV list of additional sources to compile/link, disable recursive finding with `"<dir>\, 0"` |
| `-b --binc $s`          | Add include directories to back of compile command, separated by standard system PATH environment win=`;` others=`:` |
| `-B --bpath $s`         | Add library search directory to back of link command, separated by standard system PATH environment win=`;` others=`:` |
| `-C --directory $d`     | Execute on directory `$d` rather than current directory |
| `-d --dependencies`     | See [dependency build rules](dependencies.md#dependency-build-rules) |
| `-D --dump`             | Write command logs to `./.mkn/logs/$PROFILE` |
| `-E --env $e`           | CSV key=value environment variables override format `-E "k1=v1,k2=v2"`, can be used with run/dbg, backslash escapes `,` and `=` |
| `-f --finc $s`          | Add include directories to front of compile command, separated by standard system PATH environment win=`;` others=`:` |
| `-F --fpath $s`         | Add library search directory to front of link command, separated by standard system PATH environment win=`;` others=`:` |
| `-g --debug [0-9]`      | Add compiler/linker debug flags for chosen compiler - 0 = off / 9 = full - number missing = 9 / no default |
| `-G --get $k`           | Returns string value for property K in either local yaml or settings.yaml, failure is no string 0 exit code |
| `-h --help`             | Print help |
| `-j --jargs $j`         | Takes JSON in the form of `{"c": "-DC_ARG1 -DC_ARG2", "cpp": "-DCXX_ARG1 -DCXX_ARG2"}` passing file-type specific args to compiler |
| `-l --linker $t`        | Adds `$t` to linking of root project |
| `-K --static`           | Links projects without mode as static |
| `-L --all-linker $t`    | Adds `$t` to linking of all projects |
| `-m --mod $str`         | Add module(s), see [modules](dependencies.md#add-modules-on-the-command-line) |
| `-M --main $f`          | Set main file as `$f`, overrides yaml main tag |
| `-n --nodes [$n]`       | Activate distributed compilation with `$n` nodes from chosen settings.yaml (`-x`) |
| `-o --out $f`           | Set output binary/lib name to be `$f`, overrides yaml out tag |
| `-O --optimize [0-9]`   | Add compiler/linker optimisation flags for chosen compiler - 0 = off / 9 = full - number missing = 9 / no default |
| `-p --profile $p`       | Activate profile p |
| `-P --property $p`      | CSV key=value properties override format `-P "k1=v1,k2=v2"`, backslash escapes `,` and `=` |
| `-q --quiet`            | Suppress "stale build" notice when compiling without build/compile |
| `-r --run-args $a`      | Passes `$a` to running binary as arguments, supersedes `-a` |
| `-R --dry-run`          | Print commands without executing them. |
| `-s --scm-status`       | Display SCM status of project, allows `-d` |
| `-S --shared`           | Links projects without mode as shared |
| `-t --threads $n`       | Consume `$n` threads while compiling source files where `$n > 0`, If `$n` is missing optimal resolution attempted. |
| `-T --without $CSV`     | Remove profile or dependency on the command line, see [with/out](dependencies.md#without-dependency-build-rules) |
| `-u --scm-update`       | Update project from SCM when permitted, allows `-d` |
| `-U --scm-force-update` | Force update project from SCM, allows `-d` |
| `-v --version`          | Displays the current maiken version number |
| `-V --verbose [0-5]`    | Set log level, same values as [KLOG](logging.md), number missing = 1 |
| `-w --with`             | Add profile or dependency on the command line, see [with/out](dependencies.md#without-dependency-build-rules) |
| `-W --warn`             | Add compiler warning flags for chosen compiler - 0 = off / 9 = full - number missing = 8 / no default |
| `-x --settings $f`      | Sets settings.yaml in use to `$f`, see [settings.yaml location](settings.md#location) |

## Examples

| Command | Does |
|---------|------|
| `mkn clean build -dOtug 0`             | update/clean/compile with optimal threads/link/flags in release mode |
| `mkn clean compile link -d -t -u`      | update/clean/compile with optimal threads/link everything |
| `mkn clean build -dtKa -DARG`          | clean/compile with optimal threads passing `-DARG`/link everything statically |
| `mkn clean build -dtKa "-DARG -DARG1"` | clean/compile with optimal threads passing `-DARG` and `-DARG1`/link everything statically |
| `mkn clean build -d 1 -t 2 -u`         | update/clean/compile with two threads/link project and immediate dependencies |
| `mkn -ds`                              | Display `${scm} status` for everything |
| `mkn run -- arg0 arg1 arg2`            | Execute binary passing arguments to main as argc/argv |
