# Description

This repository contains the **ULTIMETER** wind vane driver.

# Dependencies

The driver relies on:

* An external `types.h` header file defining the **standard C types** of the targeted MCU.
* The **embedded utility functions** defined in the [embedded-utils](https://github.com/Ludovic-Lesur/embedded-utils) repository.

Here is the versions compatibility table:

| **ultimeter-driver** | **embedded-utils** |
|:---:|:---:|
| [sw1.0](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.0) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |

# Compilation flags

| **Flag name** | **Value** | **Description** |
|:---:|:---:|:---:|
| `ULTIMETER_DRIVER_DISABLE_FLAGS_FILE` | `defined` / `undefined` | Disable the `ultimeter_driver_flags.h` header file inclusion when compilation flags are given in the project settings or by command line. |
| `ULTIMETER_DRIVER_DISABLE` | `defined` / `undefined` | Disable the ULTIMETER driver. |
| `ULTIMETER_DRIVER_TIMER_ERROR_BASE_LAST` | `<value>` | Last error base of the low level timer driver. |
| `ULTIMETER_DRIVER_WIND_SPEED_SAMPLING_TIME_SECONDS` | `<value>` | Time interval in seconds where the wind speed is evaluated. |
| `ULTIMETER_DRIVER_WIND_DIRECTION_SAMPLING_PERIOD_SECONDS` | `<value>` | Wind direction reading period in seconds. |

# Build

A static library can be compiled by command line with `cmake`.

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="<toolchain_file_path>" \
      -DTOOLCHAIN_PATH="<arm-none-eabi-gcc_path>" \
      -DTYPES_PATH="<types_file_path>" \
      -DEMBEDDED_UTILS_PATH="<embedded-utils_path>" \
      -DULTIMETER_DRIVER_TIMER_ERROR_BASE_LAST=0 \
      -DULTIMETER_DRIVER_WIND_SPEED_SAMPLING_TIME_SECONDS=1 \
      -DULTIMETER_DRIVER_WIND_DIRECTION_SAMPLING_PERIOD_SECONDS=10 \
      -G "Unix Makefiles" ..
make all
```
