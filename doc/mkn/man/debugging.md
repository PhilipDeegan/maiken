# Debugging

If the environment variable `MKN_DBG` is set, it is used as the process string for debugging such that the format is: `$MKN_DBG ./bin/<profile>/<binary> <args>`

Otherwise, the [settings.yaml](settings.md) variable `local->debugger` is checked, resulting in the same style of string.

If neither are set the following rules apply

| OS      | Command |
|---------|---------|
| Windows | `cdb -o ./bin/<profile>/<binary> <args>` |
| Unix    | `gdb ./bin/<profile>/<binary> <args>` |

Arguments are passed to the binary with `-r "arg0 arg1"` or after `--`, as with `run`.
If both are given, `-r` arguments come first.

Launch app with gdb, autorun and print backtrace

```sh
MKN_DBG='gdb -batch -ex run -ex bt --args' mkn dbg -r "arg0 arg1"
MKN_DBG='gdb -batch -ex run -ex bt --args' mkn dbg -- arg0 arg1
```

For LLDB try:

```sh
MKN_DBG='lldb -b -o run -o bt --' mkn dbg
```

or

```sh
MKN_DBG='lldb --batch -o run -o \"thread backtrace all\" --' mkn dbg
```
