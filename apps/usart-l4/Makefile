
# ======================================================================================#=
# MAKEFILE
# apps/usart-l4/Makefile
#
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
SRC_FILES  = apps/usart-l4/main.c
INC_DIRS   = apps/usart-l4

# ----------------------------------------------------------------------+-
# Platform
# ----------------------------------------------------------------------+-
SRC_FILES += platform/usart/usart-it-buff.c

# ----------------------------------------------------------------------+-
# Core
# ----------------------------------------------------------------------+-
SRC_FILES += core/swtrace/swtrace-led.c
INC_DIRS  += core/swtrace
INC_DIRS  += core/board

# ----------------------------------------------------------------------+-
# STM32Cube Low Level Drivers for the L4.
#
# Note: some of the #includes in the Cube CMSIS source files
# require the more specific paths shown below.
# ----------------------------------------------------------------------+-
SRC_FILES += mcu/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_utils.c
INC_DIRS  += mcu/STM32CubeL4/Drivers
INC_DIRS  += mcu/STM32CubeL4/Drivers/CMSIS/Core/Include
INC_DIRS  += mcu/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Include
INC_DIRS  += mcu/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver/Inc

# ----------------------------------------------------------------------+-
# MCU
# CMSIS Startup Init and Dependencies
# ----------------------------------------------------------------------+-
SRC_FILES += mcu/clock/cmsis-clock.c
SRC_FILES += mcu/clock/mco-stm32l4.c
SRC_FILES += mcu/clock/clock-tree-default-config-stm32l4.c

SRC_FILES += mcu/vtor/reset-handler-default-cm4.s
SRC_FILES += mcu/vtor/vector-table-gcc-stm32l476xx.s
SRC_FILES += mcu/vtor/scb.c
SRC_FILES += mcu/vtor/vtor.c



# ----------------------------------------------------------------------+-
# Linker Script
# Path to the linker script file for building this executable.
# ----------------------------------------------------------------------+-
LINKER_SCRIPT = mcu/linker-script/STM32L4-GCC-STM32L476RGTx_FLASH.ld


# ----------------------------------------------------------------------+-
# Compiler & Linker Options
#
# STM32L476xx
#     CMSIS requires this symbol be defined.
#     See: CMSIS/Device/ST/STM32L4xx/Include/stm32l4xx.h
# ----------------------------------------------------------------------+-
CFLAGS  = -g
CFLAGS += -O0
CFLAGS += -Wall
CFLAGS += -DBOARD_NUCLEO_L476RG
CFLAGS += -DMCUFAM_STM32L4
CFLAGS += -DSTM32L476xx
CFLAGS += -mlittle-endian
CFLAGS += -mthumb
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mfloat-abi=hard
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += --specs=nosys.specs

LDFLAGS  = $(CFLAGS)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,-Map=build/apps/$(APP_NAME).map


# ======================================================================================#=
# Makefile Applications Common
# Relative to the directory where this makefile is located.
# ======================================================================================#=
include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))../makefile-apps-common.mk


