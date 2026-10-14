# Rizin command reference

Verify commands with installed `rizin -h`/`<cmd>?`. Read the [PS1 playbook](rizin-playbook.md) for decisions/safety.

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

## Headless shell pattern

```bash
bin/harness analysis rizin -N -n -q -e scr.color=false -e log.quiet=true \
  -a mips -b 32 -E little -m 0x80010000 \
  -c 'aa;aac;aar;aflj;q' payload.bin
```

Use one independent Rizin invocation per JSON command or `rzpipe`; store stderr separately.

## PS1 decompiler plugin

Build/load `rz-psxdec.so` through the [workspace guide](../../../../tools/retdec-psx/README.md).
Inspect loaded `pdz?`/`pdza?`; use `Lcj` for plugin names/build versions.

| Command | Meaning |
| --- | --- |
| `pdz` | Decompile the current Rizin function |
| `pdzo` | Show code with offsets |
| `pdzj` | Print decompilation JSON |
| `pdz*` | Print commands that add decompilation comments |
| `pdza [start [end]]` / `pdzaa` | Analyze a range / all functions |
| `pdzar /absolute/workspace` | Generate from the validated fixed original-only inputs |
| `pdzaom /absolute/metadata.json image-id...` | Mount verified original images in order |
| `pdzaou image-id...` | Dismount images and reveal earlier static mappings |
| `pdzao` | Inspect the active native memory context |
| `pdze` | Show the output-directory environment setting (`DEC_SAVE_DIR`) |

Use current Rizin analysis/supplied context for ordinary function commands. Exclude
authored references from original-only inputs. Treat generated C as a hypothesis
until byte/contract/data/placement review.
The [overlay command guide](../../../../third_party/rz-psxdec/README.md#overlay-state)
owns metadata, address evidence, mount ordering and context limits.

## Runtime starting commands

```bash
bin/harness runtime status
bin/harness runtime missions
bin/harness runtime run --bios "$bios" --disc "$disc" \
  --mission "$mission" --output "$fresh_output"
```

Select embedded missions/arguments through [runtime procedures](../../psx-emulator/references/runtime.md).
Supply licensed inputs and fresh output; the harness records settings/input
identities and validates captures. Keep GDB disabled for maintained missions.
Follow [runtime analysis](runtime-and-replays.md) for authorized manual sessions.
