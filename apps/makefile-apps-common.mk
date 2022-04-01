
# ======================================================================================#=
# makefile-apps-common.mk
#
# Makefile content that is common to all apps in this project.
#
# SPDX-License-Identifier: MIT-0
# ======================================================================================#=


# ======================================================================================#=
# Makefile Common
# You should not have to change any of the following for specific apps.
# ======================================================================================#=

# ----------------------------------------------------------------------+-
# We assume a project structure like this:
#
#     ROOT_DIR
#         apps
#             APP1
#                 Makefile - to build APP1
#                 app-module
#                 app-module
#             APP2
#                 Makefile - to build APP2
#                 app-module
#                 app-module
#
#         platform
#             platform-module
#             platform-module
#
#         build
#             objs/obj-full-path
#             objs/obj-full-path
#             apps/app1.bin
#             apps/app2.bin
#
# The application name is used to name the executable binary.
# All build products are built under ROOT_DIR/build.
# ----------------------------------------------------------------------+-

# Add a / to the current directory so we can compare later.
CURDIR_DIR  = $(addsuffix /,$(CURDIR))

# Fullpath to the app specific Makefile that included this one.
# This assumes the app specific Makefile was the very first Makefile parsed by make.
APPL_MKFILE_PATH = $(abspath $(firstword $(MAKEFILE_LIST)))

# Fullpath to the application directory.
APP_DIR   = $(dir  $(APPL_MKFILE_PATH))

# The name of the app we are building.
APP_NAME  = $(notdir  $(abspath $(APP_DIR)))

# Fullpath to the project root directory.
ROOT_DIR = $(dir $(abspath $(dir $(abspath $(APP_DIR)))))

# All build artifacts are placed in this build directory.
APP_BUILD_PATH  = build/apps/$(APP_NAME)

# ----------------------------------------------------------------------+-
# Ensure that make is run from the Project Root directory.
# @@@ TODO: can make cd there?
# ----------------------------------------------------------------------+-
ifneq ($(CURDIR_DIR), $(ROOT_DIR))
$(info Makefile: $(APPL_MKFILE_PATH))
$(info For help...  )
$(info cd $(ROOT_DIR)  )
$(info make --makefile=apps/$(APP_NAME)/Makefile  help  )
$(error ERROR)
endif

# ----------------------------------------------------------------------+-
# Derive the various lists required for the build.
# ----------------------------------------------------------------------+-
SRC_FILES_ALL = $(foreach src, $(SRC_FILES), $(wildcard $(src)))
COBJ_FILES    = $(patsubst  %.c, build/objs/%.o, $(filter %.c, $(SRC_FILES_ALL)))
SOBJ_FILES    = $(patsubst  %.s, build/objs/%.o, $(filter %.s, $(SRC_FILES_ALL)))
OBJ_FILES     = $(COBJ_FILES)
OBJ_FILES    += $(SOBJ_FILES)

# Include Path
INC_DIR_OPTS  = $(foreach inc, $(INC_DIRS), $(addprefix -I,$(inc)))
# We encourage relative paths to include files when that is practical.
# For include files that are part of this project (not in a submodule)
# those paths should be relative to the Root_Dir.
INC_DIR_OPTS += -I.



# ----------------------------------------------------------------------+-
# help target
# ----------------------------------------------------------------------+-
help:
	@echo
	@echo
	@echo "Instructions to build an application:"
	@echo "    cd $(ROOT_DIR)"
	@echo "    make --makefile=apps/$(APP_NAME)/Makefile  clean"
	@echo
	@echo "Targets:"
	@echo "    all"
	@echo "    clean"
	@echo "    make-debug"


# ----------------------------------------------------------------------+-
# clean target
# ----------------------------------------------------------------------+-
clean:
	@echo
	@echo
	@echo "Removing the entire build directory..."
	$(REMOVE) $(ROOT_DIR)build
	@echo



# ----------------------------------------------------------------------+-
# all target
# ----------------------------------------------------------------------+-
all: $(APP_BUILD_PATH).bin


# ----------------------------------------------------------------------+-
# elf Target
# ----------------------------------------------------------------------+-
$(APP_BUILD_PATH).elf: $(OBJ_FILES)
	@echo
	@echo Linking: $^
	@echo    Into: $@
	@$(MKDIR) $(@D)
	$(LINK) $(LDFLAGS) -Xlinker -Map=$(APP_BUILD_PATH).map  $^  -o $@

# ----------------------------------------------------------------------+-
# bin Target
#
# When DTA_HOST is defined, this recipe will also
# deploy a copy of the binary file to the DTA:/tmp directory.
# ----------------------------------------------------------------------+-
DEPLOY_HOST = $(strip $(DTA_HOST))

$(APP_BUILD_PATH).bin: $(APP_BUILD_PATH).elf
	@echo
	@echo Generating: $@
	@echo       From: $^
	$(OBJCOPY) -O binary $^ $@
	@echo
	@if [ -z "$(DEPLOY_HOST)" ]; then echo "WARNING: Skipping deployment: DTA_HOST is not defined."; else echo "Deploying To: $(DEPLOY_HOST) "; scp $@ $(DEPLOY_HOST):/tmp/; fi
	@echo


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# Compilation Rules
#
# Implicit pattern rules to compile each source file
# into its corresponding object file in the build directory.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
build/objs/%.o: %.c
	@echo
	@echo
	@echo "Compiling: $< "
	@echo "     Into: $@ "
	@$(MKDIR) $(@D)
	$(COMPILE) $(CFLAGS)  $(INC_DIR_OPTS)  $<  -o $@

build/objs/%.o: %.s
	@echo
	@echo
	@echo "Assembling: $< "
	@echo "      Into: $@ "
	@$(MKDIR) $(@D)
	$(COMPILE) $(CFLAGS)  $(INC_DIR_OPTS)  $<  -o $@

# --------------------------------------------------------------+-
# Tools on this platform.
# --------------------------------------------------------------+-
COMPILE = arm-none-eabi-gcc -c
LINK    = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
MKDIR   = mkdir -p
REMOVE  = rm -rf



# --------------------------------------------------------------+-
# Dump the essential variables defined in this makefile.
# --------------------------------------------------------------+-
make-debug:
	@echo
	@echo CURDIR:      $(CURDIR)
	@echo APPL_MKFILE_PATH: $(APPL_MKFILE_PATH)
	@echo APP_DIR:  $(APP_DIR)
	@echo
	@echo APP_NAME:       $(APP_NAME)
	@echo ROOT_DIR:  $(ROOT_DIR)
	@echo
	@echo SRC_FILES:      $(SRC_FILES)
	@echo SRC_FILES_ALL:  $(SRC_FILES_ALL)
	@echo COBJ_FILES:  $(COBJ_FILES)
	@echo SOBJ_FILES:  $(SOBJ_FILES)
	@echo OBJ_FILES:   $(OBJ_FILES)
	@echo DTA_HOST:     $(DTA_HOST)
	@echo DEPLOY_HOST:  $(DEPLOY_HOST)


