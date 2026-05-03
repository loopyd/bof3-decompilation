# BOF3 Harness Guide

Use `bin/harness` as the small-agent entry point for BOF3 reverse/decomp work.
It records targets, claims, context, reports, and dashboard output under
`out/harness/`.

## Purpose

The harness is primarily for quick function iteration: pick one function, lift
or revise the source, run the decompile/reverse/m2c-assisted asm diff loop, and
record the evidence so another agent can continue without rediscovery.

Whole-`.bin` parity is a later stage. Use it after enough functions in a
code-bearing EMI `*.bin` have been migrated and the compiled raw `*.bin` is
ready to compare against the extracted original.

## Fast Function Loop

```bash
bin/harness status
bin/harness setup
bin/harness catalog
bin/harness analyze
bin/harness split
bin/harness queue --type function --limit 10
bin/harness claim --type function --owner "$USER"
bin/harness target init <target-id>
bin/harness context build <target-id>
bin/harness verify function <source-file>
bin/harness diff <target-id>
bin/harness finish <target-id> --status done --message "matched"
bin/harness report
bin/harness dashboard
```

When unsure, run:

```bash
bin/harness resume
```

`resume` prints one concrete next safe action.

`analyze` works without a fresh Ghidra export: it records existing lifted source
functions from `bof3/src/**/*.c` first, then adds imported Ghidra function rows
when `out/inventory/ghidra_function_index.json` exists.

## Pipeline Shortcuts

Use the harness commands for detailed target work. Use these shortcuts for
repeatable whole-workspace steps:

```bash
make harness
make harness-ready
make binary-parity
```

`make harness-ready` refreshes catalog, analysis, split targets, binary maps,
reports, and dashboard. `make binary-parity` is a later integration gate: it
extracts/uses the original code-bearing EMI `*.bin`, builds a compiler-produced
raw `*.bin`, then diffs those two raw files. It does not repack or rebuild
`.EMI` archives. The pipeline runs in allow-different mode so byte mismatches
are recorded as evidence instead of blocking the harness workflow.

## Verification Tiers

Use function parity as the active fast loop. `bin/harness verify function
<source-file>` runs the one-function compile/decompile/m2c/asm-diff lane through
the existing `asm-diff-one` implementation and writes proof artifacts under
`out/asm-diff/<function>/`.

For source-backed function targets, `bin/harness diff <target-id>` runs the same
asm-diff loop and records the result against that target.

Use binary parity after enough functions in a module are migrated. `bin/harness
verify binary <target-id>` compares the original unpacked code-bearing `.bin`
against the compiler-produced raw `.bin`; this is the later module integration
gate for a migrated payload.

## Later Binary Gate

```bash
bin/harness binary map <target-id>
bin/harness verify binary <target-id>
make binary-parity
```

This gate answers whether the whole compiled raw `*.bin` matches the original
extracted EMI `*.bin`. It is not the day-to-day function lift loop.

## Ownership Rules

- Claim before editing a target.
- Own one function or one module target at a time.
- Do not edit another active claim unless the owner hands it off.
- Use `bin/harness lock acquire ghidra --owner "$USER"` before shared Ghidra writes.
- Use `out/harness/workspaces/` for scratch work and evidence.
- Keep source changes small and reviewable.
- Run the smallest relevant diff/build check before finishing.

## Target Identity

Prefer canonical shipped identities:

- boot executable: `/boot/SLUS_004.22`
- logo executable: `/boot/LOGO/LOGO.EXE`
- EMI entries: `archive + slot`, for example `BIN/BATTLE/BATTLE.EMI#3`
- Ghidra functions: `program_path + entry_hex`

Human-facing EMI output uses symbolic kinds while preserving the raw TOC value
as `raw_type`.

## Reports

Generated harness state is ignored and lives under:

- `out/harness/harness.sqlite3`
- `out/harness/catalog.json`
- `out/harness/report.json`
- `out/harness/report.md`
- `out/harness/dashboard/index.html`

The dashboard is read-only. Treat SQLite state as the report source, not ad hoc
filesystem scans.

Generated target context lives under `out/harness/context/<target>/` and includes
editable stubs for symbols, structs, globals, and prototypes.

## Source Migration

`BATTLE.EMI#3` is the first representative source-migration proof. Its current
source remains under `bof3/src/modules/battle/03/`, and the CMake artifact
registry emits a compiler-produced raw `03.bin` for `bof3_battle_03_raw`. Do
not repack `BATTLE.EMI` for parity, and do not treat static archives such as
`03.bin.a` as final matching artifacts.

Whole-binary parity compares:

- original: `out/emi_raw/BIN/BATTLE/BATTLE/3.bin`
- compiled: `build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin`

Use `bin/harness verify binary emi:BATTLE/BATTLE#3` to record the current
binary diff state when you are ready for the module-level parity gate.

Use `bin/harness binary map emi:BATTLE/BATTLE#3` to write the companion
function/symbol/xref information file and SQLite rows. Complete maps require a
fresh Ghidra symbol export and imported function index.

For broad refreshes, use:

```bash
bin/harness binary map --all --type emi
bin/harness verify binary --all --type emi
```

The broad map pass writes one JSON map per target and refreshes the SQLite
`binary_maps`, `symbols`, and `xrefs` tables. The broad diff pass reports
`different` for compiled raw `.bin` files that exist and `missing_compiled_bin`
for source modules that have not been migrated to raw `.bin` output yet.

Use `--compiled-only` for parity sweeps that should only consider modules with
registered compiler-produced raw `.bin` output.

Use `--allow-different` when binary parity is advisory. It still fails for
missing inputs or missing compiled raw `.bin` files, but returns success after a
real original-vs-compiled comparison writes diff artifacts.

## Legacy Cleanup

The old pre-harness prompt files were removed. Use this guide plus
`bin/harness resume` for current target selection and ownership.
