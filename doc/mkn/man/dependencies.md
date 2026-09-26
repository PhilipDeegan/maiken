# Dependencies

## Dependency build rules

Using the argument `-d` / `--dep` the following rules apply

- If solely numeric, must be greater than 0, used to limit which dependencies are built,
  if 1, project immediate dependencies are built, if two, includes dependencies of dependencies.
- If non-numeric, CSV apply, with the value equating to the yaml root node "name", only those found, plus the current project are built.
- Project profiles may be specified with square brackets, e.g. `-d project[profile]`
- If no profile is chosen, all profiles from that project which would normally be active with an argumentless `-d` will be active.
- Also, if non numeric, special characters include
  - `+`, ignores current project. If only value builds all but current project.

## With/out dependency build rules

CSV with the value equating to either the current profile yaml root node "name" to include local profiles
OR - full project names as if dependencies i.e. `mkn.kul` / `org.boost`

Project profiles may be specified with square brackets, e.g. `-w project1[profile1,profile2],project2[profile1]`

If an included dependency is not from the current project, in most cases (refine) a define is added to the compilation of the current project
in the form of `-DMKN_WITH_${PROJ.ECT}` where a project name is turned uppercase and the dot is changed to an underscore

example with g++ and `-w mkn.ram` will result in a compilation command like:

```sh
g++ -DMKN_WITH_MKN_RAM -c file.obj -o file.cpp
```

A concrete git URL can be specified with a string between `()` brackets - if these are missing the project string is
attempted to be used as the URL.

Further options can be used to specify the version and the location

| Char | Meaning  |
|------|----------|
| `&`  | location |
| `#`  | version  |

such that the following commands are valid

```sh
-w mkn.ram#master&directory(https://github.com/mkn/mkn.ram)[https]
-w mkn.ram(https://github.com/mkn/mkn.ram)[https]
-w https://github.com/mkn/mkn.ram[https]
-w [profile]
```

Similarly dependencies can be removed with the `-T` option, this is for instance if you have a library installed
in your system and do not wish to use the mkn configured version.

## Add modules on the command line

Similar to `-w`/`-T`, modules may be added with `-m`, with the same rules, but with additional properties for nested attributes

```sh
mkn build -m lang.python3{compile{with:numpy}},lang.pybind11 -w lang.pybind11
```

is the same as adding the following to mkn.yaml:

```yaml
dep: lang.pybind11
mod:
- name: lang.python3
  compile:
    with: numpy
- name: lang.pybind11
```

## GitHub integration

When maiken is compiled with `-w mkn.ram[https]` features are activated when attempting to resolve dependencies when they
do not have versions specified. The formula to deduce the branch/tag are as follows:

1. if there are any releases, the most recent release is used, or
2. if there is some tags, the most recent tag is used, or
3. the default branch is queried and used,
4. finally master is used

HTTPS queries are performed to the github API to resolve these, so the remote repo URL must contain the string "github.com" in order for this to work.
Which by default it does.
