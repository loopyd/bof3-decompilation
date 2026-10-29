# Owned guest component; no game, BIOS or extraction input is consumed.
REPO ?= ../../../..
OUT ?= $(REPO)/out/audio/driver
AS = $(REPO)/bin/as
LD = $(REPO)/bin/ld
OBJCOPY = $(REPO)/bin/objcopy
NM = $(REPO)/bin/nm
SOURCES = $(addprefix kernel/,startup.s cd.s events.s exceptions.s interrupts.s syscalls.s callbacks.s) $(addprefix audio/,spu.s tick.s voices.s configuration.s transfers.s reverb.s)
OBJECTS = $(addprefix $(OUT)/,$(SOURCES:.s=.o))

.PHONY: bof3-audio-driver clean
.DELETE_ON_ERROR:
bof3-audio-driver: $(OUT)/kernel.bin $(OUT)/kernel.sym

$(OUT):
	mkdir -p "$@"

$(OUT)/%.o: %.s | $(OUT)
	mkdir -p "$(@D)"
	"$(AS)" -EL -mips1 -G0 -o "$@" "$<"

$(OUT)/kernel.elf: $(OBJECTS) kernel/layout.ld
	"$(LD)" -EL -T kernel/layout.ld -Map "$(OUT)/kernel.map" -o "$@" $(OBJECTS)

$(OUT)/kernel.bin: $(OUT)/kernel.elf
	"$(OBJCOPY)" -O binary "$<" "$@"

$(OUT)/kernel.sym: $(OUT)/kernel.elf
	"$(NM)" -n --defined-only "$<" > "$@"

clean:
	rm -f $(OBJECTS) "$(OUT)/kernel.elf" "$(OUT)/kernel.bin" "$(OUT)/kernel.sym" "$(OUT)/kernel.map"
