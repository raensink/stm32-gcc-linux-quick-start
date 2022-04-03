
# ======================================================================================#=
# MAKEFILE
# apps/button-blinky-it/Makefile
#
# Builds the blinky app for the STM32F0 on a NUCLEO-F091RC board.
#
# SPDX-License-Identifier: MIT-0
# ======================================================================================#=

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# SOURCE FILES
# INCLUDE DIRECTORIES
#
# This application consists of the source files listed below.
# We also list the include paths where the header file dependencies can be satisfied.
# All file paths are relative to the project root directory.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

# ----------------------------------------------------------------------+-
# Application
# ----------------------------------------------------------------------+-
SRC_FILES  = apps/button-blinky-it/main.c
INC_DIRS   = apps/button-blinky-it

# ----------------------------------------------------------------------+-
# STM32Cube Low Level Drivers for the F0.
#
# Note: some of the #includes in the Cube CMSIS source files
# require the more specific paths shown below.
# ----------------------------------------------------------------------+-
SRC_FILES += mcu/STM32CubeF0/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_utils.c
INC_DIRS  += mcu/STM32CubeF0/Drivers
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Core/Include
INC_DIRS  += mcu/STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Include
INC_DIRS  += mcu/STM32CubeF0/Drivers/STM32F0xx_HAL_Driver/Inc

# ----------------------------------------------------------------------+-
# Platform
# ----------------------------------------------------------------------+-
# None required for this application;

# ----------------------------------------------------------------------+-
# Core
# ----------------------------------------------------------------------+-
SRC_FILES += core/swtrace/swtrace-led.c

# ----------------------------------------------------------------------+-
# MCU
# CMSIS Startup Init and Dependencies
# ----------------------------------------------------------------------+-
SRC_FILES += mcu/clock/cmsis-clock.c
SRC_FILES += mcu/clock/clock-tree-default-config-stm32f0.c
SRC_FILES += mcu/clock/mco-stm32f0.c

SRC_FILES += mcu/vtor/reset-handler-default-cm0.s
SRC_FILES += mcu/vtor/vector-table-gcc-stm32f091xc.s

### TODO? SRC_FILES += mcu/stm32f0/core/scb.c


# ----------------------------------------------------------------------+-
# Linker Script
# ----------------------------------------------------------------------+-
LINKER_SCRIPT = mcu/linker-script/STM32F0-GCC-STM32F091RCTx_FLASH.ld

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


