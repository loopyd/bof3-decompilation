# BOF3 Harness Guide

Use `bin/harness` as the small-agent entry point for BOF3 reverse/decomp work.
It records targets, claims, context, reports, and dashboard output under
`output/harness/`.

## Purpose

The harness is primarily for quick function iteration: pick one function, lift
or revise the source, run the decompile/reverse/m2c-assisted asm diff loop, and
record the evidence so another agent can continue without rediscovery.

Whole-`.bin` parity is a later stage. Use it after enough functions in a
code-bearing EMI `*.bin` have been migrated and the compiled raw `*.bin` is
ready to compare against the extracted original.

## Fast Function Loop

```bash
bin/bootstrap --plan
bin/harness refresh
bin/harness status
bin/harness candidates --module emi:ETC/GAME#0 --min-size 512 --limit 10
bin/harness claim --module emi:ETC/GAME#0 --owner "$USER"
bin/harness lift <target-id>
bin/harness verify function <source-file>
bin/harness report function <target-id-or-source>
bin/harness finish <target-id> --status done --message "matched"
bin/harness report summary
```

When unsure, run:

```bash
bin/harness status
```

`refresh` is a cheap harness-state refresh, not a Ghidra run. The expensive
Ghidra import/analysis/export belongs to project setup and should normally be
done once per extracted binary set with `bin/bootstrap`, or repeated
only after binaries, Ghidra scripts, loader behavior, or symbol-export rules
change. `refresh` records existing lifted source functions from
`bof3/src/**/*.c`, then adds game-code rows from
`output/inventory/ghidra_function_index.json`.

Imported GTE/BIOS/PsyQ/library rows are support symbols. Keep them in maps and
context, but do not claim them as game reverse targets.

Ghidra headless is single-writer for the shared project. Normal function work
must not run Ghidra. Use existing exports through `make lift-ready`,
`bin/harness refresh`, and the lift/verify commands. If the project must be
refreshed, use the locked harness wrappers:

```bash
bin/harness ghidra import-project --no-analysis
bin/harness ghidra analyze
bin/harness ghidra export
```

The `ghidra-ready`, `decomp-ready`, and `decomp-full-ready` pipelines use those
wrappers. Do not call `bin/ghidra-import-project` or `bin/ghidra-export-symbols`
directly from a small-agent loop.

## Pipeline Shortcuts

Use the harness commands for detailed target work. Use these shortcuts for
repeatable whole-workspace steps:

```bash
make decomp-full-ready
make lift-ready
make harness
make harness-ready
```

`make decomp-full-ready` is the canonical full refresh: extraction, EMI unpack,
inventory, one Ghidra project import without analysis, serialized Ghidra
analysis, Ghidra symbol export, inventory import, Ghidra coverage check, and
harness refresh.

`make lift-ready` and `make harness-ready` are cheap repeatable refreshes before
function work. They run `bin/harness refresh` and do not run Ghidra.

## Verification Tiers

Use `bin/harness lift <target-id>` for the normal target preparation loop. It is
the context build and m2c path, and prints the original asm
and m2c draft paths.

Use function parity as the active fast loop. `bin/harness verify function
<source-file>` runs the one-function compile/decompile/m2c/asm-diff lane through
the existing `asm-diff-one` implementation and writes proof artifacts under
`output/asm-diff/<function>/`.

Use `bin/harness verify module <module> --allow-different` to check every
source-backed function in a module and get a compact match-percent table. This
is the quickest way to find the next source function to improve.

## Ownership Rules

- Claim before editing a target.
- Own one function or one module target at a time.
- Do not edit another active claim unless the owner hands it off.
- Shared Ghidra project writes must go through `bin/harness ghidra ...` wrappers.
- Use `output/harness/workspaces/` for scratch work and evidence.
- Keep source changes small and reviewable.
- Use PsyQ headers through `bof3/include/bof3/psyq_compat.h`.
- Run the smallest relevant diff/build check before finishing.

## Target Identity

Prefer canonical shipped identities:

- boot executable: `/boot/SLUS_004.22`
- logo executable: `/boot/LOGO/LOGO.EXE`
- EMI entries: `archive + slot`, for example `BIN/BATTLE/BATTLE.EMI#3`
- Ghidra functions: `program_path + entry_hex`

For day-to-day function work, prefer the human alias accepted by `bin/harness`:

```text
func:<archive>#<slot>@<address>
func:ETC/GAME#0@0x801ba678
func:BMAGIC/MAGIC121#3@0x801ef414
```

These aliases resolve to extracted raw EMI payloads such as
`output/extracted/BIN/ETC/GAME/0.bin`. Staged Ghidra names like
`GAME_e00_80195800.bin` are import/project names only; they keep the single
Ghidra project collision-free and load-address-visible.

Human-facing EMI output uses symbolic kinds while preserving the raw TOC value
as `raw_type`.

## Reports

Generated harness state is ignored and lives under:

- `output/harness/harness.sqlite3`
- `output/harness/catalog.json`
- `output/harness/report.json`
- `output/harness/report.md`
- `output/harness/dashboard/index.html`

The dashboard is read-only. Treat SQLite state as the report source, not ad hoc
filesystem scans.

Generated target context lives under `output/harness/context/<target>/` and includes
editable stubs for symbols, structs, globals, and prototypes.

## Source Migration

`BATTLE.EMI#3` is the first representative source-migration proof. Its current
source remains under `bof3/src/modules/battle/03/`, and the CMake artifact
registry emits a compiler-produced raw `03.bin` for `bof3_battle_03_raw`. Do
not repack `BATTLE.EMI` for parity, and do not treat static archives such as
`03.bin.a` as final matching artifacts.

## Legacy Cleanup

The old pre-harness prompt files were removed. Use this guide plus
`bin/harness candidates`, `claim`, `lift`, and `verify` for current target
selection and ownership.
