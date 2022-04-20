
# ======================================================================================#=
# MAKEFILE
# apps/freertos/Makefile
#
# SPDX-License-Identifier: MIT-0
# ======================================================================================#=


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# SOURCE FILES
# INCLUDE DIRECTORIES
# Identify the source files and include paths.
# All file paths are relative to the project root directory.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

# ------------------------------------------------------+-
# Application
# ------------------------------------------------------+-
INC_DIRS   = apps/freertos
SRC_FILES  = apps/freertos/main.c

# ------------------------------------------------------+-
# FreeRTOS
# ------------------------------------------------------+-
SRC_FILES += core/FreeRTOS-Kernel/tasks.c
SRC_FILES += core/FreeRTOS-Kernel/queue.c
SRC_FILES += core/FreeRTOS-Kernel/list.c
SRC_FILES += core/FreeRTOS-Kernel/timers.c
SRC_FILES += core/FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c
SRC_FILES += core/FreeRTOS-Kernel/portable/MemMang/heap_1.c

INC_DIRS  += core/FreeRTOS-Kernel/include
INC_DIRS  += core/FreeRTOS-Kernel/portable/GCC/ARM_CM0


# ------------------------------------------------------+-
# MCU: STM32CubeF0
# CMSIS Core, Low Level Drivers, and other device specifics
# Note: some of the #includes in the CMSIS source files
# require the more specific paths shown below.
# ------------------------------------------------------+-
INC_DIRS  += mcu/STM32CubeF0/Drivers
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Core/Include
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Include
INC_DIRS  += mcu/STM32CubeF0/Drivers/STM32F0xx_HAL_Driver/Inc

# ------------------------------------------------------+-
# CMSIS
# ------------------------------------------------------+-
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Include
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Include

SRC_FILES += mcu/STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c
SRC_FILES += mcu/STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f091xc.s

# ------------------------------------------------------+-
# Vector Table and Exception Handlers
# ------------------------------------------------------+-
### ### SRC_FILES += mcu/stm32f0/vtor/reset-handler.s
### ### SRC_FILES += mcu/stm32f0/vtor/vector-table.s

# CMSIS Startup Init and Dependencies
### INC_DIRS  += mcu/stm32f0/
### SRC_FILES += mcu/stm32f0/core/scb.c
### SRC_FILES += mcu/stm32f0/core/clock.c

# ------------------------------------------------------+-
# Platform
# ------------------------------------------------------+-
# None for this application;

# ----------------------------------------------------------------------+-
# Linker Script
# Path to the linker script file for building this executable.
# ----------------------------------------------------------------------+-
LINKER_SCRIPT = mcu/stm32f0/STM32F091RCTx_FLASH.ld

# ----------------------------------------------------------------------+-
# Compiler & Linker Options
#
# STM32F091xC
#     CMSIS requires this symbol be defined.
#     See: CMSIS/Device/ST/STM32F0xx/Include/stm32f0xx.h
# ----------------------------------------------------------------------+-
CFLAGS  = -g
CFLAGS += -O0
CFLAGS += -Wall
CFLAGS += -DSTM32F091xC
CFLAGS += -DSTM32F0XX
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m0
CFLAGS += --specs=nosys.specs

LDFLAGS  = $(CFLAGS)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,-Map=build/apps/$(APP_NAME).map


# ======================================================================================#=
# Makefile Applications Common
# Relative to the directory where this makefile is located.
# ======================================================================================#=
include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../makefile-apps-common.mk


