PROFILE ?= compat/capcom97

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= $(ROOT)/build
PROFILE_DIR := $(ROOT)/config/compiler-profiles/$(PROFILE)

# Load profile defaults; validate existence.
ifeq ($(wildcard $(PROFILE_DIR)/default.mk),)
  $(error unsupported PROFILE=$(PROFILE); expected $(PROFILE_DIR)/default.mk)
endif
include $(PROFILE_DIR)/default.mk
-include $(PROFILE_DIR)/exe.mk
-include $(PROFILE_DIR)/psyq.mk
-include $(PROFILE_DIR)/overlays.mk
-include $(PROFILE_DIR)/exceptions.mk

# Compose final flag variables.  Profile files define _base variables;
# the Makefile adds repository-wide -I paths and fixed wrappers.
CPPFLAGS ?= $(CPPFLAGS_base) -I$(ROOT)/src -I$(ROOT)/include \
            -I$(ROOT)/toolchains/psyq/4.7/include
CFLAGS   ?= $(CFLAGS_base)
CC_ASFLAGS ?= $(CC_ASFLAGS_base)
ASFLAGS  ?= $(ASFLAGS_base)

CC := $(ROOT)/bin/cc
AS := $(ROOT)/bin/as

C_SOURCES := $(shell find $(ROOT)/src -type f -name '*.c' -print | sort)
ASM_SOURCES := $(shell find $(ROOT)/src -type f \( -name '*.s' -o -name '*.S' \) -print | sort)
SHARED_INPUTS := $(shell find $(ROOT)/src/shared -type f \( -name '*.h' -o -name '*.inc' \) -print 2>/dev/null | sort)
C_OBJECTS := $(patsubst $(ROOT)/src/%.c,$(BUILD_DIR)/src/%.o,$(C_SOURCES))
ASM_OBJECTS := $(foreach source,$(ASM_SOURCES),$(BUILD_DIR)/src/$(patsubst $(ROOT)/src/%,%,$(basename $(source))).o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

# Per-source-group CFLAGS overrides from profile files.
# Each overrides target is a no-op when the _exe/_psyq/_overlays variable
# is empty (the common case for profiles with uniform flags).
$(BUILD_DIR)/src/exe/%.o: CFLAGS += $(CFLAGS_exe)
$(BUILD_DIR)/src/emi/%.o: CFLAGS += $(CFLAGS_emi)
$(BUILD_DIR)/src/shared/%.o: CFLAGS += $(CFLAGS_shared)

.DEFAULT_GOAL := all

all: $(OBJECTS)

$(BUILD_DIR)/src/%.o: $(ROOT)/src/%.c $(SHARED_INPUTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CC_ASFLAGS) -c $< -o $@

$(BUILD_DIR)/src/%.o: $(ROOT)/src/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/src/%.o: $(ROOT)/src/%.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)/src

.PHONY: all clean
