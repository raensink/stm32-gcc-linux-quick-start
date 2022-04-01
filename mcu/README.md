# stm32-gcc-linux-quick-start

## MCU Folder
Startup code and low-level drivers specific to a particular microcontroller.

#### mcu/linker-script
GCC linker scripts for a variety of STM32 microcontrollers.

#### mcu/STM32CubeF0
Submodule that references:
[https://github.com/STMicroelectronics/STM32CubeF0](https://github.com/STMicroelectronics/STM32CubeF0)

Here are the instructions for re-creating this submodule (if you must).
(These instructions assume you are using Linux.)
```
cd  stm32-gcc-linux-quick-start  # project root
git submodule add  https://github.com/STMicroelectronics/STM32CubeF0.git  ./mcu/STM32CubeF0
```


