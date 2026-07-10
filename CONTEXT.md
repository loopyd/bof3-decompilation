# BOF3 Context

Canonical language for the BOF3 multi-binary decompilation workspace.

## Binaries

- **Disc input**: user-owned US BIN/CUE media in `disks/`; never tracked.
- **PS-X executable**: a header-wrapped executable on disc, including
  `SLUS_004.22` and `LOGO.EXE`.
- **Load image**: the headerless raw image extracted from a PS-X executable.
  Splat and binary matching use this image; PS-X EXE header metadata is kept
  separately.
- **Executable target**: a tracked standalone binary layout under
  `config/splat/exe/` with source under `src/exe/`.

## EMI

- **EMI archive**: a shipped `.EMI` container. It is not a build or
  decompilation target.
- **EMI entry**: one payload extracted from an archive, identified by its disc
  archive path and slot, for example `BIN/BATTLE/BATTLE.EMI#3`.
- **Payload kind**: catalog classification: `ram`, `image`, `audio`, or
  `unresolved`. It is not a claim that an entry is executable.
- **Code status**: review state: `unknown`, `candidate`, `confirmed`, or
  `rejected`. Explicit review and promotion are required for `confirmed`.
- **Promoted target**: a confirmed code or mixed code/data entry with a tracked
  Splat configuration and source directory beneath `src/emi/`.

## Identity and evidence

- **Content group**: entries with the same payload SHA-256, regardless of
  runtime address.
- **Build target**: payload SHA-256, load address, and entry convention.
  Identical bytes loaded at different addresses remain separate targets until
  relocatability and symbol behaviour are proven.
- **Catalog**: generated local evidence in `out/catalog/`, including entry
  facts, classification, and duplicate groups. It is regenerated, not authored.
- **Durable layout**: tracked Splat configurations and symbol files in
  `config/splat/` and `config/symbols/`. Generated analysis output is not
  durable layout state.

## Source and analysis boundaries

- `src/exe/` owns standalone executable source; `src/emi/` owns promoted EMI
  source.
- A lifted function is one `func_XXXXXXXX.c` file; target-local declarations
  belong in its adjacent `internal.h`.
- PsyQ routines are library code: declare and use them, but do not lift them.
- `out/` is the only generated-artifact root: extraction, normalized images,
  catalogs, Splat products, Ghidra state, drafts/diffs, and asset previews.
