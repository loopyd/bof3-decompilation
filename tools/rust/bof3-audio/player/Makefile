# Run in the generated compile output directory. REPO selects the full checkout.
REPO ?= ../../../..
CC = $(REPO)/bin/cc
AS = $(REPO)/bin/as
LD = $(REPO)/bin/ld
OBJCOPY = $(REPO)/bin/objcopy
CFLAGS = -O2 -G0 -msoft-float -fno-builtin -fno-pic -ansi -pedantic -Wall -W

.PHONY: all clean
all: player.exe

player.o: player.c config.h
	PSX_GCC="$(REPO)/toolchains/gcc-2.7.2-psx/gcc" "$(CC)" $(CFLAGS) -c player.c -o $@
start.o: start.s
	"$(AS)" -EL -mips1 -G0 -o $@ $<
assets.o: assets.s bank.vh bank.vb song.sep runtime.bin font.bin palette.bin logo.bin
	"$(AS)" -EL -mips1 -G0 -o $@ $<
player.elf: start.o player.o assets.o player.ld
	"$(LD)" -EL -T player.ld -Map player.map -o $@ start.o player.o assets.o
player.exe: player.elf
	"$(OBJCOPY)" -O binary $< $@
clean:
	rm -f player.o start.o assets.o player.elf player.exe player.map
