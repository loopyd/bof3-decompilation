# PSX input mapping

Analyze the runtime image, not whichever container happened to carry it. Record
the input hash, byte range, and verified runtime address before creating a
project. Original bytes and tracked target metadata outrank analyzer guesses.

## Choose the input

| Input | On-disc shape | Analyzer input | Address source |
| --- | --- | --- | --- |
| PS-X EXE | `0x800`-byte `PS-X EXE` header followed by code/data | The normalized, headerless load image under `out/binaries/exe/` | Little-endian `t_addr` at header offset `0x18`; validate `t_size` at `0x1c` and initial PC at `0x10` |
| Normalized executable | Bytes copied from PS-X EXE offset `0x800`, with no executable header or relocations | The normalized file itself | Owning target manifest, cross-checked against the original EXE header |
| EMI archive | Header, entry table, alignment, and multiple unrelated payloads | **Never** the whole archive | Not applicable |
| Extracted EMI entry | Headerless raw payload for one archive slot | The promoted target image under `out/binaries/emi/` | Reviewed entry RAM pointer plus the owning target manifest |

A PS-X EXE header describes a flat load: bytes after offset `0x800` are loaded
at `t_addr`; it is not a relocatable executable. Do not map the wrapper at
`t_addr`, because that shifts every runtime byte by `0x800`. Do not strip bytes
from an EMI entry unless its reviewed format says they are container metadata.
An EMI type or plausible pointer is discovery evidence, not sufficient proof
that an entry is code.

For BOF3, resolve the input through the harness instead of copying files into an
analysis directory:

```sh
bin/harness target list
bin/harness analysis doctor
bin/harness analysis init exe/slus_004_22
bin/harness analysis init emi/etc/game/00
```

The target manifest supplies the canonical normalized path and load address.
If the original PS-X EXE header disagrees with the manifest, stop and correct
the tracked manifest after review; do not compensate inside Rizin.

## Record identity and bounds

Before analysis, retain enough evidence to reproduce the mapping:

```sh
sha256sum out/binaries/exe/slus_004_22.bin
stat -c '%n %s bytes' out/binaries/exe/slus_004_22.bin
xxd -g4 -l 64 out/extracted/SLUS_004.22
```

For a PS-X EXE, verify:

- bytes `0x00..0x07` are `PS-X EXE`;
- `pc0` is the little-endian word at `0x10`;
- `t_addr` is the little-endian word at `0x18`;
- `t_size` is the little-endian word at `0x1c` and does not exceed the bytes
  available after the `0x800`-byte header;
- the normalized image equals the intended `t_size` payload bytes, allowing
  only a documented normalization policy;
- `t_addr <= pc0 < t_addr + normalized_size` before treating `pc0` as an entry.

For an EMI entry, record archive path, slot, payload SHA-256, exact extracted
size, reviewed load address, and the source of every proposed entry point.
Require each code/data address to satisfy:

```text
load_address <= address < load_address + payload_size
file_offset = address - load_address
```

Reject wrapped, truncated, misaligned, or cross-entry ranges. Hash-identical
payloads at different load addresses remain different analysis targets until
relocatability and symbol behavior are proven.

## Map a raw runtime image

Raw PSX images carry no architecture metadata. Set 32-bit MIPS and little
endian explicitly, and use `-m` to map the file at its runtime load address:

```sh
rizin -a mips -b 32 -e cfg.bigendian=false -m 0x80096800 \
  out/binaries/exe/slus_004_22.bin

r2 -a mips -b 32 -e cfg.bigendian=false -m 0x80195800 \
  out/binaries/emi/etc/game/00.bin
```

Use `-m` for these headerless raw images. `-B` changes the base used by the
binary-format loader (notably PIE/parsed binaries); it does not replace raw-file
mapping. Mixing them can yield convincing disassembly at the wrong addresses.

The PlayStation CPU is a little-endian MIPS R3000A/MIPS I target. CPU-profile
names vary by engine/plugin version, so inspect the available values before
setting one; do not invent a profile name:

```text
e asm.arch
e asm.bits
e cfg.bigendian
e asm.cpu=?
```

The required observed state is `asm.arch=mips`, `asm.bits=32`, and
`cfg.bigendian=false`. If an installed backend offers an applicable MIPS I or
R3000 profile, select it and re-check representative instructions. Otherwise
leave the backend's MIPS default and record that limitation. A decompiler CPU
model does not override the raw bytes or runtime map.

## Validate before auto-analysis

Confirm the map and known entry point before running broad analysis:

```text
ij
om
s 0xENTRY
px 32
pd 12
```

Check that `om` covers exactly the expected runtime interval and that `px`
matches the source bytes at `ENTRY - load_address`. For a PS-X EXE, start with
the verified `pc0`. An EMI payload has no universal entry field: use a reviewed
entry table, known call target, Splat symbol, or caller evidence and label its
provenance. Delay `aaa` until branches, delay slots, aligned words, and nearby
absolute references look like credible PSX MIPS code.

After analysis, sanity-check boundaries and references rather than accepting
all discovered functions:

```text
aaa
afl
afij @ 0xENTRY
axt @ 0xENTRY
```

Analyzer-created functions, names, types, and entry points are hypotheses.
Replay only reviewed facts from `config/analysis/`, then verify them against
canonical disassembly or raw bytes.

## Project isolation

Create one generated project per target identity: normalized input hash, load
address, and entry convention. Never reuse a project merely because two files
have equal bytes. Address-based flags, xrefs, types, and function boundaries
become invalid when the same payload is mapped elsewhere.

Use the harness so Rizin/radare2 version differences and project paths remain
centralized:

```sh
bin/harness analysis init TARGET
bin/harness analysis query TARGET functions
bin/harness analysis export TARGET
```

Projects belong under `out/analysis/projects/`; deterministic exports belong
under `out/analysis/exports/`. Reviewed replay commands belong under
`config/analysis/`. Never treat disposable project state as the only copy of a
name or type decision.

## Sources

- [Rizin Handbook: command-line options](https://book.rizin.re/src/first_steps/commandline_options.html)
  documents `-a`, `-b`, `-B`, `-m`, projects, and configuration assignments.
- [Official radare2 book: mapping files](https://book.rada.re/commandline/mapping_files.html)
  distinguishes raw-file mapping with `-m` from binary-loader rebasing with
  `-B`.
- [Official radare2 book: firmware setup](https://book.rada.re/r2fwrev/setup.html)
  covers explicit architecture, CPU, bitness, endian, load address, entry flag,
  and reproducible setup for raw images.
- [PSX-SPX: CD-ROM file formats](https://psx-spx.consoledev.net/cdromfileformats/)
  documents the PS-X EXE header, `0x800` header size, initial PC, destination
  address, payload size, and flat loading behavior.
- [PSX-SPX: Kernel/BIOS](https://psx-spx.consoledev.net/kernelbios/)
  confirms that the loader copies the executable body to the address specified
  by the EXE header.
