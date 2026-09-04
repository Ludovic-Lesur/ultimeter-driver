# Description

This repository contains the **ULTIMETER** wind vane driver.

# Dependencies

The driver relies on:

* An external `types.h` header file defining the **standard C types** of the targeted MCU.
* The **embedded utility functions** defined in the [embedded-utils](https://github.com/Ludovic-Lesur/embedded-utils) repository.

Here is the versions compatibility table:

| **ultimeter-driver** | **embedded-utils** |
|:---:|:---:|
| [sw2.0](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw2.0) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |
| [sw1.4](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.4) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |
| [sw1.3](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.3) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |
| [sw1.2](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.2) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |
| [sw1.1](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.1) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |
| [sw1.0](https://github.com/Ludovic-Lesur/ultimeter-driver/releases/tag/sw1.0) | >= [sw7.0](https://github.com/Ludovic-Lesur/embedded-utils/releases/tag/sw7.0) |

> [!WARNING]
> The low level timer must have a minimum period of 13 seconds, the recommended configuration is more than 30s with a 500µs step. Once started, it is advised to keep the timer running with its natural overflow, to ensure a good measurements continuity (the overflow event is internally managed by the driver).

# Compilation flags

| **Flag name** | **Value** | **Description** |
|:---:|:---:|:---:|
| `ULTIMETER_DRIVER_DISABLE_FLAGS_FILE` | `defined` / `undefined` | Disable the `ultimeter_driver_flags.h` header file inclusion when compilation flags are given in the project settings or by command line. |
| `ULTIMETER_DRIVER_DISABLE` | `defined` / `undefined` | Disable the ULTIMETER driver. |
| `ULTIMETER_DRIVER_TIMER_ERROR_BASE_LAST` | `<value>` | Last error base of the low level timer driver. |
| `ULTIMETER_DRIVER_TIMER_STEP_US` | `<value>` | Timer counter step in microseconds. |

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
      -DULTIMETER_DRIVER_TIMER_STEP_US=1000 \
      -G "Unix Makefiles" ..
make all
```
