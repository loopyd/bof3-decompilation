# Owned audio guest driver

This target builds authored little-endian MIPS I assembly and its linker layout.
It needs GNU Make, `sha256sum` and the approved `mipsel-none-elf` GNU binutils.
The full repository defaults to its existing `toolchains/psyqo/bin/` tools.
An extracted source package uses the same target with an explicit tool prefix:

```sh
make -C tools/rust/bof3-audio/driver bof3-audio-driver \
  CROSS_COMPILE=/path/to/bin/mipsel-none-elf-
```

If those tools are already on `PATH`, use `CROSS_COMPILE=mipsel-none-elf-`.
Command-line `AS`, `LD`, `OBJCOPY` and `NM` values override the corresponding
tools individually. Building does not install tools or read a game executable,
BIOS, disc image or extraction cache.

Outputs default to `out/audio/driver/` beneath the repository or package root.
`OUT` can select another directory; keep it relative to the driver directory
when its containing path has spaces. The target produces `kernel.elf`,
`kernel.map`, `kernel.bin`, `kernel.sym` and `kernel.json`. The runtime needs the
last three files together. The `bof3.audio-driver/v1` manifest declares
`mips1-le-o32` and hashes the exact binary and defined-nm symbol bytes.

The low image, mutable state and stacks occupy the reserved
`0x80020000..0x80050000` arena. The initializer copies the packed 628-byte
scheduler to `0x8016C548..0x8016C7BC`; reserve that execution range separately.
The linker checks both allocations. Source comments retain original US function
ranges, data identities and provenance hashes.

The Rust [`machine::driver::Image`](../src/machine/driver.rs) validates artifact
hashes, required symbols and allocation bounds, then starts services in zeroed
RAM with explicit event, exception, timer, CD, DMA and SPU bindings. It uses the
1,024-byte initialization stack and separate 4,096-byte exception stack.
After service initialization, the host selects GP zero, foreground SP
`0x801FFF00` and status `0x401`, with automatic PAD acknowledgement disabled.
Sound initialization, layout selection and host-fed bank/SEP transfer follow
as separate guest calls. No original ROM or game installation is required.

The production PSX music renderer uses this driver. Other production operations,
full SFX/vocal contexts, native timing and independent PCM acceptance remain open.
The authored sources use the project [MIT license](../LICENSE); dependency
notices remain in [`THIRD_PARTY_LICENSES.txt`](../THIRD_PARTY_LICENSES.txt).
