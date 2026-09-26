# Environment variables

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `KLOG` | Number | `0` | See [logging](logging.md) |
| `MKN_COMPILE_THREADS` | uint | none | Global override, if set forces all compile calls to use value. Example, low RAM systems |
| `MKN_DEFAULT_BRANCH` | String | `"master"` | Default branch for SCM if not given or set |
| `MKN_DBG` | String | `""` | Sets the preceding command line string when using "dbg". See [debugging](debugging.md) |
| `MKN_OBJ` | String | Linux/BSD=`o`, Windows=`obj` | Sets the file type to compile object files to |
| `MKN_GCC_PREFERRED` | bool | `false` | When attempting automatic settings.yaml creation, use gcc even if cl/clang are found |
| `MKN_CL_PREFERRED` | bool | `false` | Windows only: when attempting automatic settings.yaml creation, use cl even if clang/gcc are found |
| `MKN_LIB_EXT` | string | Windows=`dll`, others=`so` | The file extension of shared objects being created. |
| `MKN_LIB_PRE` | string | Windows=`""`, others=`lib` | The file prefix of shared objects being created. |
| `MKN_LD_PRELOAD` | string | not set | Forwards `MKN_LD_PRELOAD` as `LD_PRELOAD` for run/dbg |
