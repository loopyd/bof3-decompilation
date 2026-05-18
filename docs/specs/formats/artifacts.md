# Disk And Artifact Spec

This page describes the current repo-owned artifact model used by inventory,
Ghidra, matching, and the harness.

## Generated Roots

- `output/extracted/`: local extracted disc files and unpacked EMI archives
- `output/inventory/`: generated inventory JSON and Markdown views
- `output/ghidra-bof3/`: generated Ghidra import manifest inputs
- `output/harness/`: generated decomp harness state, reports, workspaces, and dashboard

Generated roots are not durable source. Refresh them with `bin/` commands.

## Artifact Identity

Use shipped identity first:

| Artifact | Identity |
| --- | --- |
| boot executable | `/boot/SLUS_004.22` |
| logo executable | `/boot/LOGO/LOGO.EXE` |
| EMI entry | `BIN/<family>/<archive>.EMI#<slot>` |
| imported function | `<program_path>@<entry_hex>` |
| source migration target | `migration:<name>` |

## EMI Symbolic Kinds

Harness and report output preserves raw EMI TOC values as `raw_type` and adds a
human-facing symbolic kind:

| Symbolic kind | Raw type | Meaning |
| --- | ---: | --- |
| `EMI_BINARY_RAM` | `0` | generic CPU RAM payload, often code or data |
| `EMI_LARGE_RAM_BLOB` | `1` or large type `0` | large CPU RAM blob with mixed-content risk |
| `EMI_IMAGE_VRAM` | `3` | raw image or VRAM-oriented payload |
| `EMI_AUDIO_VH` | `6` | PSX VAB header |
| `EMI_AUDIO_VB` | `7` | PSX VAB body |
| `EMI_AUDIO_AUX` | `8` | audio-side auxiliary payload |
| `EMI_AUDIO_SEQ` | `10` | PSX sequence payload |
| `EMI_UNKNOWN` | other | unmapped or not yet proven |

Classifier output must include an explanation so reviewers can see why a target
was queued or deprioritized.

## Harness State

`bin/harness refresh` records EMI entries, build artifacts, configured migration
targets, and existing lifted source functions under
`bof3/src/**/*.c` and also imports Ghidra function rows when
`output/inventory/ghidra_function_index.json` exists.

Use `bin/harness ghidra ...` before shared Ghidra project writes. The harness
takes a SQLite `ghidra` lease under `output/harness/harness.sqlite3`; it
coordinates agents, not operating-system processes.

## First Migration Proof

`BIN/BATTLE/BATTLE.EMI#3` is registered as `bof3_battle_03_raw` and emits a
compiler-produced raw `03.bin` from source objects. The harness does not repack
or rebuild `.EMI` archives for parity. Static archives such as `03.bin.a` are
build intermediates, not final matching artifacts.

Whole-binary parity compares the original unpacked code payload against the
compiled raw payload:

| Role | Path |
| --- | --- |
| original | `output/extracted/BIN/BATTLE/BATTLE/3.bin` |
| compiled | `build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin` |

This is a later-stage integration check. The fast decompile loop is
function-level parity through `bin/harness verify function <source-file>` or
`bin/asm-diff-one <source-file>`.

Each code-bearing `.bin` should also have a map record:

- file: `output/harness/binary-maps/<target>/binary-map.json`
- database tables: `binary_maps`, `symbols`, and `xrefs`

The map records functions from `output/inventory/ghidra_function_index.json`, from
compiled raw-module metadata when present, and symbols/xrefs from
`output/inventory/raw_ghidra_export.json` when those generated inputs exist.

`bin/harness lift <target>` writes context, original asm, and m2c draft outputs
under `output/harness/workspaces/<target>/` and
`output/harness/context/<target>/`.

For the named workflow, use `make harness-ready` to refresh state. The Makefile
target is a thin alias; `bin/harness` remains the source of state and evidence.
