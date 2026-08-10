# Rizin playbook for PS1 binaries

> In this repo prefer the wired `bin/rz-project` wrapper: `bin/rz-project analyze TARGET`, `bin/rz-project status TARGET`, `bin/rz-project open TARGET` (writes `out/reverse/snapshots/<encoded-target>.json`). The generic `rizin` invocations below are the skill-local fallback outside that workspace.

## Canonical raw mapping

Extract a PS-X EXE payload or identify a raw overlay base, then:

```bash
rizin -a mips -b 32 \
  -e cfg.bigendian=false \
  -m 0x80010000 \
  -i assets/rizin/psx-init.rz \
  payload.bin
```

`-m` maps a raw file at the proven runtime address. Confirm installed CLI (`rizin -h`); option behavior evolves.

## Staged analysis

Order: `aa` (roots) → `aar` (data refs) → `aaf` (call targets) → `aac` (calls from focus) → `aad` (ptr-to-ptr). Use `aaa` only after checking ranges/boundaries — fully automated analysis can be nonsensical.

```text
e analysis.hasnext=false
e analysis.jmp.indir=true
e analysis.jmp.tbl=true
e analysis.datarefs=true
e analysis.refstr=true
e analysis.strings=true
```

Raw mixed files: set analysis ranges where possible; inspect `e analysis.in=??`.

## Navigation

| Cmd | Meaning |
|---|---|
| `s <addr>` | seek |
| `pdf @ <func>` | function disassembly |
| `pd 20 @ <addr>` | 20 instructions |
| `pD 80 @ <addr>` | 80 bytes as disassembly |
| `px 64 @ <addr>` | 64-byte hex dump |
| `iz / izz / izj / izzj` | strings; `izzj` JSON for raw images |

## Functions

| Cmd | Meaning |
|---|---|
| `af @ <addr>` | analyze function |
| `afu <end> @ <start>` | resize/reanalyze through end |
| `af+ <name> ...` | handcraft when needed |
| `afb @ <addr>` | list basic blocks |
| `afi / afij` | function information |
| `afn <name> @ <addr>` | rename function |
| `afs` | show/edit signature (`afs?`) |

Plausible prologue alone ≠ proof; direct/runtime edges are stronger.

## Calls / xrefs

| Cmd | Meaning |
|---|---|
| `axt / axtj @ <addr>` | xrefs to |
| `axf / axfj @ <addr>` | xrefs from |
| `axg @ <addr>` | graph paths reaching target |
| `axl / axlj` | all xrefs |
| `axC <target> @ <from>` | add call xref |
| `axc / axd / axs <target> @ <from>` | add code/data/string xref |

Add manual xrefs only after recording how the target was derived.

## Variables / arguments

`afvl` list · `afva` analyze · `afv=` accesses. Check `afv?`, `afc?`, `afs?` before scripted mutations. Argument recovery is a caller/callee/runtime exercise; command output is one source.

## Hints (control flow wrong)

| Cmd | Meaning |
|---|---|
| `ahc <target> @ <addr>` | override jump/call target |
| `ahd <opcode> @ <addr>` | override displayed opcode |
| `ahs <size> @ <addr>` | override opcode size |
| `ahl` | list hints |

Prefer hints over global analysis when one instruction/table is the issue.

## GP / jump tables

```text
e analysis.gp=<value>
e analysis.gpfixed=true|false
```

Experimental per the official handbook; GP may differ per function. Manual reconstruction remains necessary for overlay-relative, relocated, compressed, or script dispatch tables.

## Flags / comments / namespaces

`f name @ <addr>` · `fr old new` · `fs <space>` · `fC name comment`. Namespaces: `main.*`, `ovl.<id>.*`, `psyq.*`, `bios.*`, `data.*`, `trace.*`. Never erase original symbol spelling; add normalized aliases.

## Types

`td <declaration>` define · `to <header>` load · `ts` structs · `tp <type> @ <addr>` print as type. Consult `t?`, `afc?`. Apply types after the offset ledger is coherent; verify propagation didn't hide contradictory machine-code behavior.

## Decompiler (rz-ghidra)

`pdg` decompile · `pdgo` side-by-side with offsets · `pdgj` JSON · `pdgx` XML · `pdg*` comments · `pdgs` languages. Pin rz-ghidra to a tag compatible with the installed Rizin release. Compare `pdg` vs `pdf` + runtime.

## Signatures

FLIRT workflows: `F` command family + `rz-sign` (check `F?`/`rz-sign -h`; paths/subcommands evolve). Create patterns from symbolized libraries, load, apply; record false positives and functions too short to identify safely.

## JSON / automation

Prefer JSON for durable tooling: `ij`, `aflj`, `axlj`, `izzj`, `afij`, `axtj`, `axfj`, `pdgj`. Bundled scripts use independent Rizin invocations to reduce interactive-state ambiguity. High-volume work: `rzpipe` + command logs + raw JSON.

## Function artifact minimum

Per function directory:

```text
metadata.json
disassembly.txt
decompile.txt
decompile-offsets.txt
decompile.json
xrefs-to.json
xrefs-from.json
variables.txt
notes.md
```

Regenerate artifacts after boundary, symbol, type, or xref changes; diff the directories.
