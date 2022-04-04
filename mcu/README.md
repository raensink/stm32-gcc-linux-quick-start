
# stm32-gcc-linux-quick-start

## MCU Folder
Startup code and low-level drivers specific to a particular microcontroller.

#### mcu/clock
Processor specific code to support clock tree configuration.

#### mcu/linker-script
GCC linker scripts for a variety of STM32 microcontrollers.

#### mcu/vtor
Processor specific vector table and default reset handler.


#### mcu/STM32CubeF0
Submodule that references:
[https://github.com/STMicroelectronics/STM32CubeF0](https://github.com/STMicroelectronics/STM32CubeF0)

The size of this repository is beyond enormous!
We need only a tiny fraction of the files contained therein.
We do include, however, the entire repo as a submodule just so we can be more certain
about referencing an officially released version of those few files we do care about.
This is probably an interim solution until we create our own copy/fork of
the official STM32CubeF0 repo where we can safely pare down the content to just what we need.

Here are the instructions for re-creating this submodule (if you must).
These instructions assume you are using Linux.
```
cd  stm32-gcc-linux-quick-start  # project root
git submodule add  https://github.com/STMicroelectronics/STM32CubeF0.git  ./mcu/STM32CubeF0
```


#### mcu/STM32CubeL4
Submodule that references:
[https://github.com/STMicroelectronics/STM32CubeL4](https://github.com/STMicroelectronics/STM32CubeL4)

As is the case with the F0 Cube, the size of this repository is also beyond enormous!
And once again we need only a tiny fraction of the files contained therein.

Here are the instructions for re-creating this submodule (if you must).
These instructions assume you are using Linux.
```
cd  stm32-gcc-linux-quick-start  # project root
git submodule add  https://github.com/STMicroelectronics/STM32CubeL4.git  ./mcu/STM32CubeL4
```


