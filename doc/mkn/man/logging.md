# Logging

| `KLOG`    | Logging to std out |
|-----------|--------------------|
| [missing] | NONE  |
| `1`       | INFO  |
| `2`       | ERROR |
| `3`       | DEBUG |
| `4`       | OTHER |
| `5`       | TRACE |

## Activating logging

Windows:

```bat
SET KLOG=3
mkn clean build
```

Linux/Mac:

```sh
KLOG=3 mkn clean build
```

Or with the `-V` argument, which takes the same values and overrides `KLOG`:

```sh
mkn clean build -V 3
```
