# Command reference

> In this repo prefer wired `bin/rz-project`, `bin/asm-diff`, `bin/byte-match`, `bin/permute`. The Rizin summaries below are a generic reference for work outside `out/reverse/snapshots/<encoded-target>.json`.

## Essentials

| Cmd | Meaning |
|---|---|
| `?` / `<cmd>?` | help |
| `s <address>` | seek |
| `pd <instructions>` | disassemble instruction count |
| `pD <bytes>` | disassemble byte count |
| `pdf` | function disassembly |
| `px <bytes>` | hex |
| `izzj` | raw strings JSON |
| `aflj` | functions JSON |
| `axlj` | xrefs JSON |

## Analysis

`aa` baseline roots · `aaa` advanced auto-analysis (inspect results) · `aab` basic-block analysis · `aaf` all function calls · `aac` calls from selected/current function · `aar` data references · `aad` pointers-to-pointers · `af` analyze function · `afu <end>` resize function.

## Xrefs

| Cmd | Meaning |
|---|---|
| `axt / axtj` | to current address |
| `axf / axfj` | from current address |
| `axg` | reachability graph |
| `axC <target>` | add call xref from seek |
| `axc <target>` | add code xref |
| `axd <target>` | add data xref |
| `axs <target>` | add string xref |

## Variables / types

`afvl` list vars/args · `afva` analyze vars/args · `afv=` variable accesses · `afs` function signature · `td` define C type · `to` load C header · `ts` structs · `tp` print typed data.

## rz-ghidra

`pdg` decompile · `pdgo` offsets + decompile · `pdgj` JSON · `pdgx` XML · `pdgs` languages.

## Headless shell pattern

```bash
rizin -q -a mips -b 32 -e cfg.bigendian=false -m 0x80010000 \
  -c 'aa;aac;aar;aflj;q' payload.bin
```

Production scripts: one JSON command per controlled invocation or `rzpipe`; store stderr separately.

## Runtime starting commands

PCSX-Redux GDB examples use interpreter/debugger/GDB startup + a client connecting to `localhost:3333`. Confirm current flags in installed PCSX-Redux.

```bash
gdb-multiarch
(gdb) set architecture mips
(gdb) target remote localhost:3333
```

Record emulator CLI/settings in the case rather than relying on GUI state.
