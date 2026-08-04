# Reverse-engineering lessons

> Durable cross-cutting gotchas that make the BOF3 lift-and-match loop faster and safer.

Domain contracts belong in `docs/specs/`; repeatable procedures in the owning
operating reference. This file keeps concrete findings easy to repeat or
misdiagnose across targets. Matching/permuter procedures:
[function matching](matching.md).

## Evidence and boundaries

### Establish payload boundaries before lifting

- Splat labels are reviewed inputs, not stronger evidence than payload bytes.
- A plausible decoded instruction can still be embedded data: in
  `emi/world00/area008/13`, bytes at payload offset `0x14` begin with the
  `"%d"` entry header; code starts at `0x18`.
- Check calls, saved return addresses, return paths are coherent: a false
  start at `0x801f2c14` appeared to call before its prologue and would have
  returned into itself.
- Split confirmed leading data in Splat before promoting the real boundary.

### Normalize analyzer addresses against payloads

- Raw EMI payloads may contain a header before the configured code VRAM.
  Reconcile analyzer addresses with payload offsets and the Splat segment
  start before promoting a boundary. Canonical payload bytes and tracked
  layouts outrank analyzer-created function names.
- Never load an extracted `GAME.EMI` entry as one linear raw image at its
  first function address. Entry 0 begins with a count/pointer header. Entry 1
  loads at `0x801d0c00`, begins with a control word, reaches the title setup
  handler only at payload offset `0x90` (`0x801d0c90`); using that handler as
  load address shifts every payload offset. Normalize and split through the
  target's tracked Splat layout, then verify against canonical lift assembly.
- `GAME.EMI#0` loads at `0x80195800`; its first reviewed function is at
  payload offset `0x91c` (`0x8019611c`). Configuring the target at the first
  function silently shifts byte reads and analyzer addresses by `0x91c`.
  Sequence-based asm resolution can still report plausible or exact matches
  under that bad base — a green function diff does not validate the load
  address. Cross-check the catalog/header destination; require
  `runtime address - load address == payload offset` before adding boundaries.

### Account for runtime state and cross-target pointers

- Frontend callback tables at `0x801c7b08`/`0x801c7b14` are zero in the
  shipped SLUS load image, populated at runtime. Recover consumers/producers
  from code/xrefs; zero-filled EXE bytes are not evidence of absence.
- A callback table owned by one EMI payload may intentionally target the
  concurrently loaded companion overlay: `GAME.EMI#0` tables mix local
  `0x8019...` with `0x801d...`/`0x801e...` targets. Preserve the pointer as a
  reviewed table entry; do not create a local boundary or reject the table
  because the target lies outside the payload map.

## Executable metadata

- Read PS-X EXE `t_addr` from header offset `0x18`; never assume the common
  `0x80010000` base. `SLUS_004.22` loads at `0x80096800`.
- A wrong manifest base maps valid runtime addresses into unrelated zero
  padding: the EMI loader at `0x80161f58` sits at normalized-image offset
  `0xcb758`; subtracting the former `0x80010000` base gave the false offset
  `0x151f58` and an apparent all-zero library.
- Cross-check the tracked manifest against the normalized binary's generated
  metadata and the original PS-X EXE header before concluding code is
  runtime-generated or missing.
- Apply the check independently per PS-X executable: `LOGO.EXE` loads at
  `0x801ce000`; a common-base `0x80010000` reading puts its entry point and
  reviewed functions outside the normalized payload.

## Build and matching

### Diagnose toolchain failures before changing candidate C

- If `bin/asm-diff TARGET@0xADDRESS` cannot compile a new lift, diff one
  known existing function from the same target; the same failure on both means
  a workspace/toolchain problem, not wrong candidate C.
- A compiler exit without diagnostics is not a comparison result. Preserve the
  last verified diff; fix the compile path before tuning source shape.
- The historical compiler is a statically linked 32-bit i386 executable; under
  a managed sandbox it can exit `225`/`159` before processing arguments, even
  `--version`. Re-run `bin/cc` with its approved out-of-sandbox permission;
  never add flags or rewrite C to address the exit.

### Force global read/store order with a local when m2c reorders

- m2c commonly reorders independent global accesses: `flag = 2;
  counter += 0x14;` may emit the constant-load and `sb` before the `lhu` read
  of `counter`, breaking byte-match despite matching semantics.
- Pin the original load-then-store order with a local: `count = counter;
  flag = 2; counter = (u16)(count + 0x14);`. Keep the narrow-width cast so the
  store width matches. Byte-matched `func_801F4578`/`func_801F3258`
  (0x8014932A/0x80149333 pair) across `emi/world00/area008/13` and
  `emi/world00/area026/13`.

### Preserve fixed-RAM pointer ownership before permuting source shape

- When m2c exposes a fixed-RAM address as a pointer-valued `D_XXXXXXXX`
  global, add the raw symbol to the target-local map and declare its narrowest
  evidence-backed type in `internal.h`; never hide a known RAM global behind
  an anonymous address macro.
- Match qualifiers to the observed contract: an unjustified `volatile` pointee
  changes register allocation and moves stores across comparisons;
  `func_800B2218` matched only after `D_80148648` became a named
  `PanelTask*`. Add `volatile` only with asynchronous/hardware-mutation
  evidence.
- Recover stable field offsets into a target-local struct before permuting.
  Addresses, masks, encoded values stay hexadecimal; human quantities (32-pixel
  step, 320-pixel clamp) decimal.

### Share duplicate behavior, not target ownership

- Exact bytes make a strong source-shape reuse candidate; they do not make
  one function address, extern declaration, or semantic provenance global.
- "Has authored C" ≠ "has a matching lift": a 7.95% representative was
  rejected despite exact group bytes; percentage ranks effort, never
  authorizes propagation.
- Validate a second member independently before sharing code, so target-local
  compiler context cannot become hidden plumbing.
- Normalize equivalent variables and struct fields before extracting a shared
  body; divergent local vocabulary only hides unresolved understanding.
- Use a semantic `src/shared/<domain>/*.inc` body after two independently
  matching cross-target members when reuse offsets the indirection. Keep
  address-based wrappers so each target compiles/validates its own symbol; no
  runtime wrapper call merely to remove repeated source text.

### Matching technique reference

Register-pinning ladder, `MATCHING_AID`, `barrier()`/`CLOBBER_*`, pointer
hoisting, table indexing: [matching playbook](matching-playbook.md) and
[memory API](memory-api.md).

### Reach fixed RAM through `PSX_PTR`/`PSX_REF`, never raw casts or `vu8`

- All fixed-address access goes through `include/memory/` and
  `include/base/`. The `vu8`/`vu16`/`vu32` typedefs are gone; write
  `volatile u8`/`u16`/`u32` directly on the `type` argument
  (`PSX_REF(volatile u16, 0x80143B90u)`).
- Scratchpad RAM (`0x1F800000`): `SPAD_ADDR`/`SPAD_REF`/`SPAD_PTR_SLOT`. Use
  `PSX_REF(volatile type, address)` only when an access must stay volatile.
  See the [memory API](memory-api.md).

### Keep `SPAD_PTR_SLOT` cell non-volatile to match constant-address codegen

- A `volatile` slot forces `lui + ori + lw`, breaking the match. To force a
  per-evaluation reload, use explicit volatile pointer-cell
  `PSX_REF(Entity * volatile, SPAD_ADDRESS(off))`.

## Target ownership and symbols

- Check the shared SDK maps (`config/sdk/psyq-*.txt`) before adding a symbol
  to a target-local map: Splat composes both files, and a name defined in
  both aborts `bin/splat` with "Duplicate symbol detected". Keep the entry in
  exactly one map.
- PsyQ code can be linked more than once at different addresses across
  executables and EMI payloads; an address verified in `SLUS_004.22` is not a
  contract for another binary.
- Use official PsyQ names; record the verified archive member in the owning
  target's symbol map. Generated weak bindings take the runtime address from
  that map.
- Replace analyzer aliases only after behavior and signature are proven. Raw
  `func_XXXXXXXX`/`D_XXXXXXXX` names are replaced directly by a reviewed
  semantic name; no compatibility aliases.
- Preserve pre-promotion evidence with an `INFERRED:` comment beside the
  owning address-based declaration: what was observed, what would verify
  promotion. Never create a semantic alias from a hint alone.
- Compare target-qualified analyzer snapshots under
  `out/reverse/<target>/snapshot.json`; equal addresses across targets are
  insufficient. Overlays and PsyQ copies can share a role with different
  addresses or bytes.
