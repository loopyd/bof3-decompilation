PROFILE ?= compat/capcom97

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= $(ROOT)/build

ifeq ($(PROFILE),compat/capcom97)
  CPPFLAGS ?= -DHARNESS_TARGET_PSX=1 -I$(ROOT)/include -I$(ROOT)/toolchains/psyq/4.7/include
  CFLAGS ?= -O2 -G0 -funsigned-char -msoft-float -gcoff
  ASPSX_VERSION ?= 2.56
else
  $(error unsupported PROFILE=$(PROFILE); only compat/capcom97 is implemented)
endif

CC := $(ROOT)/bin/cc
AS := $(ROOT)/bin/as

# Keep compiler and assembler flags separate. bin/cc consumes the -Wa values
# while bin/as receives the same assembler options for original .s sources.
CC_ASFLAGS ?= -Wa,--aspsx-version=$(ASPSX_VERSION) -Wa,-G0,-EL,-mips1
ASFLAGS ?= -G0 -EL -mips1

C_SOURCES := $(shell find $(ROOT)/src -type f -name '*.c' -print | sort)
ASM_SOURCES := $(shell find $(ROOT)/src -type f \( -name '*.s' -o -name '*.S' \) -print | sort)
C_OBJECTS := $(patsubst $(ROOT)/src/%.c,$(BUILD_DIR)/src/%.o,$(C_SOURCES))
ASM_OBJECTS := $(foreach source,$(ASM_SOURCES),$(BUILD_DIR)/src/$(patsubst $(ROOT)/src/%,%,$(basename $(source))).o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

.DEFAULT_GOAL := all

all: $(OBJECTS)

$(BUILD_DIR)/src/%.o: $(ROOT)/src/%.c
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
