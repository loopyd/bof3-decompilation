# Owned guest component; no game, BIOS or extraction input is consumed.
REPO ?= ../../../..
OUT ?= $(REPO)/out/audio/driver
CROSS_COMPILE ?= $(REPO)/toolchains/psyqo/bin/mipsel-none-elf-
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
NM = $(CROSS_COMPILE)nm
SOURCES = $(addprefix kernel/,startup.s cd.s events.s exceptions.s interrupts.s syscalls.s callbacks.s timers.s) $(addprefix audio/,spu.s tick.s voices.s configuration.s transfers.s reverb.s initialization.s commands.s controls.s parameters.s notes.s startup.s layout.s banks.s body.s sequences.s scheduler.s effects.s status.s cues.s)
OBJECTS = $(addprefix $(OUT)/,$(SOURCES:.s=.o))

.PHONY: bof3-audio-driver clean
.DELETE_ON_ERROR:
bof3-audio-driver: $(OUT)/kernel.bin $(OUT)/kernel.sym $(OUT)/kernel.json

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

$(OUT)/kernel.json: $(OUT)/kernel.bin $(OUT)/kernel.sym
	image_hash=$$(sha256sum < "$(OUT)/kernel.bin") && \
	symbols_hash=$$(sha256sum < "$(OUT)/kernel.sym") && \
	printf '{"schema":"bof3.audio-driver/v1","abi":"mips1-le-o32","image_sha256":"%s","symbols_sha256":"%s"}\n' "$${image_hash%% *}" "$${symbols_hash%% *}" > "$@"

clean:
	rm -f $(OBJECTS) "$(OUT)/kernel.elf" "$(OUT)/kernel.bin" "$(OUT)/kernel.sym" "$(OUT)/kernel.json" "$(OUT)/kernel.map"
