# Run in the generated compile output directory. REPO selects the full checkout.
REPO ?= ../../../..
CC = $(REPO)/bin/cc
AS = $(REPO)/bin/as
LD = $(REPO)/bin/ld
OBJCOPY = $(REPO)/bin/objcopy
CFLAGS = -O2 -G0 -msoft-float -fno-builtin -fno-pic -ansi -pedantic -Wall -W
OBJECTS = start.o player.o audio.o bank.o sfx.o vocal.o controls.o graphics.o hud.o assets.o
HEADERS = psx.h audio.h bank.h sfx.h vocal.h controls.h graphics.h hud.h config.h $(wildcard banks.h)

.PHONY: all clean
all: player.exe

%.o: %.c $(HEADERS)
	PSX_GCC="$(REPO)/toolchains/gcc-2.7.2-psx/gcc" "$(CC)" $(CFLAGS) -c $< -o $@
start.o: start.s
	"$(AS)" -EL -mips1 -G0 -o $@ $<
assets.o: assets.s banks.s runtime.s $(wildcard *.vh *.vb *.sep *.aux) runtime.bin font.bin palette.bin logo.bin
	"$(AS)" -EL -mips1 -G0 -o $@ $<
player.elf: $(OBJECTS) player.ld
	"$(LD)" -EL -T player.ld -Map player.map -o $@ $(OBJECTS)
player.exe: player.elf
	"$(OBJCOPY)" -O binary $< $@
clean:
	rm -f $(OBJECTS) player.elf player.exe player.map
