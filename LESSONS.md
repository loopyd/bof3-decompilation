# Reverse-engineering lessons

> Durable cross-cutting gotchas that make the BOF3 lift-and-match loop faster and safer.

Domain contracts belong in `docs/specs/`; repeatable procedures belong in the
owning operating reference. This file retains concrete findings that are easy
to repeat or misdiagnose across targets. Use the
[`matching workflow`](docs/matching.md) for matching and permuter procedures.

## Evidence and boundaries

### Establish payload boundaries before lifting

- Treat Splat labels as reviewed inputs, not stronger evidence than payload
  bytes.
- A plausible decoded instruction can still be embedded data. In
  `emi/world00/area008/13`, bytes at payload offset `0x14` begin with the
  `"%d"` entry header; executable code starts at offset `0x18`.
- Check that calls, saved return addresses, and return paths are coherent. A
  false start at `0x801f2c14` appeared to call before its prologue and would
  have returned into itself.
- Split confirmed leading data explicitly in Splat before promoting the real
  function boundary.

### Normalize analyzer addresses against payloads

- Raw EMI payloads may contain a header before the configured code VRAM.
  Reconcile analyzer addresses with payload offsets and the Splat segment start
  before promoting a boundary. Canonical payload bytes and tracked layouts
  remain authoritative over analyzer-created function names.
- Do not load an extracted `GAME.EMI` entry as one linear raw image at its
  first function address. Entry 0 begins with a count/pointer header. Entry 1
  loads at `0x801d0c00`, begins with a control word, and does not reach the
  title setup handler until payload offset `0x90` (`0x801d0c90`). Using that
  handler as the target load address shifts every payload offset. Normalize and
  split the entry through the target's tracked Splat layout, then verify
  boundaries against canonical lift assembly.
- `GAME.EMI#0` loads at `0x80195800`; its first reviewed function is at payload
  offset `0x91c` (`0x8019611c`). Configuring the target at the first function
  silently shifts direct byte reads and analyzer addresses by `0x91c`.
  Sequence-based asm resolution can still report plausible or exact function
  matches under that bad base, so a green function diff does not validate the
  target load address. Cross-check the catalog/header destination and require
  `runtime address - load address == payload offset` before adding boundaries.

### Account for runtime state and cross-target pointers

- Frontend callback tables at `0x801c7b08` and `0x801c7b14` are zero in the
  shipped SLUS load image and populated at runtime. Recover their consumers and
  producers from code/xrefs; do not treat static zero-filled EXE bytes as
  evidence that the callbacks are absent.
- A callback table owned by one EMI payload may intentionally contain targets
  in the concurrently loaded companion overlay. `GAME.EMI#0` tables mix local
  `0x8019...` targets with `0x801d...`/`0x801e...` targets. Preserve the pointer
  as a reviewed table entry, but do not create a local function boundary or
  reject the table merely because the target lies outside the payload map.

## Executable metadata

- Read PS-X EXE `t_addr` from header offset `0x18`; do not assume the common
  `0x80010000` base. `SLUS_004.22` loads at `0x80096800`.
- A wrong target-manifest base can map valid runtime addresses into unrelated
  zero padding. The EMI loader at `0x80161f58` is present at normalized-image
  offset `0xcb758`; subtracting the former `0x80010000` manifest base produced
  the false offset `0x151f58` and an apparent all-zero library.
- Cross-check the tracked target manifest against the normalized binary's
  generated metadata and the original PS-X EXE header before concluding that
  code is runtime-generated or missing.
- Apply the same check independently to every PS-X executable. `LOGO.EXE`
  loads at `0x801ce000`; treating it as a common-base `0x80010000` image puts
  its real entry point and reviewed functions outside the normalized payload.

## Build and matching

### Diagnose toolchain failures before changing candidate C

- If `bin/asm-diff TARGET@0xADDRESS` cannot compile a new lift, diff one known
  existing function from the same target. The same failure on both functions
  indicates a workspace or toolchain problem, not evidence that the candidate
  C is wrong.
- A compiler exit without diagnostics is not a comparison result. Preserve the
  last verified diff and fix the compile path before tuning source shape.
- The historical compiler is a statically linked 32-bit i386 executable. Under
  a managed sandbox it can exit `225` or `159` before processing arguments,
  even for `--version`. Re-run the repository `bin/cc` driver with its approved
  out-of-sandbox permission; do not add flags or rewrite C to address the exit.

### Preserve fixed-RAM pointer ownership before permuting source shape

- When m2c exposes a fixed-RAM address as a pointer-valued `D_XXXXXXXX`
  global, add that raw symbol to the target-local map and declare its narrowest
  evidence-backed type in the target `internal.h`. Do not hide a known RAM
  global behind an anonymous address macro.
- Match qualifiers to the observed contract. An unjustified `volatile`
  pointee can change register allocation and move stores across comparisons;
  `func_800B2218` matched only after `D_80148648` became a named
  `BattleLocalPanelTask*`. Add `volatile` only when asynchronous or hardware
  mutation is part of the evidence.
- Recover stable field offsets into a target-local struct before trying
  permutations. Keep addresses, masks, and encoded values hexadecimal; write
  human quantities such as the 32-pixel step and 320-pixel clamp in decimal.

### Share duplicate behavior, not target ownership

- Exact bytes make a strong source-shape reuse candidate. They do not make one
  function address, extern declaration, or semantic provenance global.
- Normalize equivalent variables and struct fields before extracting a shared
  body. A shared implementation with divergent local vocabulary only hides
  unresolved understanding.
- Use a compile-time `.inc` body after two independently matching members.
  Keep address-based wrappers so each target still compiles and validates its
  own symbol; do not introduce a runtime wrapper call merely to remove repeated
  source text.

### Keep equivalence-test output isolated

- Extractor parity tests must create a unique directory under `/tmp` and remove
  only that exact directory. Never use repository `out/` as scratch output or a
  cleanup root: it contains the user's extracted media and retained local
  reverse-engineering evidence.
- Resolve and validate the temporary path before cleanup, install a scoped
  trap, and reject empty, root, repository, or repository-`out` cleanup targets.

### Do not synthesize an executable link model

- A partial set of lifted SLUS objects is a validation archive, not a rebuilt
  PS-X executable. Do not invent a CRT entry point, linker layout, or probe loop
  to make it link.
- `LOGO.EXE` is independently loaded. A SLUS helper that copies its streaming
  loop and calls LOGO-local addresses crosses the binary ownership boundary;
  preserve such investigation evidence only outside the compiled SLUS source
  set.

## Target ownership and symbols

- PsyQ library code can be linked more than once at different addresses across
  executables and EMI payloads. An address verified in `SLUS_004.22` is not a
  shared address contract for another binary.
- Use official PsyQ function names and record the verified archive member in
  the owning target's symbol map. Generated weak bindings receive the runtime
  address from that map.
- Replace analyzer aliases only after behavior and signature are proven. Raw
  `func_XXXXXXXX`/`D_XXXXXXXX` map names are replaced directly by a reviewed
  semantic name; do not maintain compatibility aliases.
- Preserve useful pre-promotion evidence with an `INFERRED:` comment beside the
  owning address-based declaration. State what was observed and what would
  verify promotion; do not create a semantic alias from a hint alone.
- Compare target-qualified analyzer snapshots under
  `out/reverse/<target>/snapshot.json`; equal addresses across targets are not
  enough evidence. Overlays and PsyQ copies can share a role while having
  different addresses or bytes.
