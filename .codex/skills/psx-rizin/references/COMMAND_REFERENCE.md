# Rizin command reference

Verify against installed `rizin -h` and `<cmd>?`; commands evolve. For PS1 decision/safety rules see [RIZIN_PLAYBOOK.md](RIZIN_PLAYBOOK.md).

## Core

| Cmd | Meaning |
|---|---|
| `?` / `<cmd>?` | help |
| `s <addr>`; `s+<delta>` / `s-<delta>` | seek absolute/relative |
| `pd <instructions>` / `pD <bytes>` | disassemble by instruction/byte count; e.g. `pd 20 @ <addr>`, `pD 80 @ <addr>` |
| `pdf`; `pdf @ <func>` | current/selected function disassembly |
| `px <bytes>` | hex; e.g. `px 64 @ <addr>` |
| `iz` / `izz` / `izj` / `izzj` | strings; raw/all and JSON variants; use `izzj` for raw-image JSON |
| `ij` / `aflj` / `axlj` | core information/functions/xrefs JSON |

## Analysis

`aa` roots · `aab` basic blocks · `aar` refs · `aaf` calls · `aac` calls from focus · `aad` pointers · `aaa` broad auto-analysis.

## Xrefs

| Cmd | Meaning |
|---|---|
| `axt / axtj`; `axt / axtj @ <addr>` | xrefs to current/selected address |
| `axf / axfj`; `axf / axfj @ <addr>` | xrefs from current/selected address |
| `axg`; `axg @ <addr>` | paths reaching current/selected target |
| `axl / axlj` | all xrefs |
| `axC <target>`; `axC <target> @ <from>` | add call xref from current/selected address |
| `axc <target>`; `axc <target> @ <from>` | add code xref from current/selected address |
| `axd <target>`; `axd <target> @ <from>` | add data xref from current/selected address |
| `axs <target>`; `axs <target> @ <from>` | add string xref from current/selected address |

## Functions

`af @ <addr>` analyze · `afu <end> @ <start>` resize/reanalyze through end · `af+ <name> ...` handcraft · `afb @ <addr>` basic blocks · `afi / afij` info · `afn <name> @ <addr>` rename · `afs` signature (`afs?`).

## Variables / types

`afvl` list vars/args · `afva` analyze · `afv=` accesses · `td <declaration>` define type · `to <header>` load header · `ts` structs · `tp <type> @ <addr>` typed data. Help: `afv?`, `afc?`, `afs?`, `t?`.

## GP and signatures

`e analysis.gp=<value>` set GP · `e analysis.gpfixed=true|false` control fixed GP. FLIRT/`rz-sign`: create patterns from symbolized libraries → load → apply; use `F` + `rz-sign`; check `F?` and `rz-sign -h` as paths/subcommands evolve.

## Hints, flags, and comments

`ahc <target> @ <addr>` override call/jump target · `ahd <opcode> @ <addr>` override displayed opcode · `ahs <size> @ <addr>` override opcode size · `ahl` list · `f name @ <addr>` flag · `fr old new` rename · `fs <space>` flagspace · `fC name comment` comment.

## rz-ghidra

`pdg` decompile · `pdgo` offsets + decompile · `pdgj` JSON · `pdgx` XML · `pdg*` comments · `pdgs` languages.

## Headless shell pattern

```bash
rizin -q -e scr.color=false -e log.quiet=true \
  -a mips -b 32 -m 0x80010000 \
  -c 'aa;aac;aar;aflj;q' payload.bin
```

Production scripts: one independent Rizin invocation per JSON command (reduces interactive-state ambiguity), or `rzpipe`; store stderr separately.

## Runtime starting commands

```bash
pcsx-redux -iso game.cue
bizhawk --movie scenario.bk2 game.cue
duckstation-qt -batch -fullscreen game.cue
gdb-multiarch
(gdb) set architecture mips
(gdb) target remote localhost:3333
```

Confirm installed flags; emulator CLIs change. Record emulator CLI/settings in the case rather than relying on GUI state.
