
# ======================================================================================#=
# MAKEFILE
# apps/button-blinky-it/Makefile
#
# SPDX-License-Identifier: MIT-0
# ======================================================================================#=

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# SOURCE FILES
# INCLUDE DIRECTORIES
# Here we identify the source files and include paths.
# All file paths are relative to the project root directory.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

# ------------------------------------------------------+-
# Application
# ------------------------------------------------------+-
SRC_FILES  = apps/button-blinky-it/main.c

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


# STM32 HAL Low Level Drivers
SRC_FILES += mcu/STM32CubeF0/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_utils.c


# CMSIS Exception Handlers
SRC_FILES += mcu/stm32f0/vtor/vector-table.s

# CMSIS Startup Init and Dependencies
INC_DIRS  += mcu/stm32f0/
SRC_FILES += mcu/stm32f0/core/scb.c
SRC_FILES += mcu/stm32f0/core/clock.c

# ------------------------------------------------------+-
# Platform
# ------------------------------------------------------+-
# None for this application;

# ----------------------------------------------------------------------+-
# MCU
# ----------------------------------------------------------------------+-
SRC_FILES += mcu/vtor/reset-handler-cm0-default.s

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


