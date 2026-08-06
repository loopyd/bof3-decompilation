# Reverse-engineering lessons

Domain contracts belong in `docs/specs/`; repeatable procedures in the owning
operating reference. Matching levers: [matching playbook](matching-playbook.md);
iteration procedure: [function matching](matching.md).

## Evidence and boundaries

### Establish payload boundaries before lifting

- Splat labels are reviewed inputs, not stronger evidence than payload bytes.
  A plausible decoded instruction can still be embedded data
  (`emi/world00/area008/13`: `"%d"` header at payload `0x14`, code at
  `0x18`). Check calls, saved return addresses, and return paths are
  coherent; split confirmed leading data in Splat before promoting the real
  boundary.

### Normalize analyzer addresses against payloads

- Raw EMI payloads may contain a header before the configured code VRAM.
  Reconcile analyzer addresses with payload offsets and the Splat segment
  start before promoting a boundary; canonical payload bytes and tracked
  layouts outrank analyzer-created function names.
- Never load an extracted `GAME.EMI` entry as one linear raw image at its
  first function address. Entry 0 begins with a count/pointer header; entry 1
  reaches its title setup handler only at payload offset `0x90`. `GAME.EMI#0`
  loads at `0x80195800`, first reviewed function at payload `0x91c` — a
  target configured at the first function silently shifts every byte read,
  and sequence-based asm resolution can still report exact matches under that
  bad base. Require `runtime address - load address == payload offset`
  before adding boundaries.
### Account for runtime state and cross-target pointers

- Frontend callback tables at `0x801c7b08`/`0x801c7b14` are zero in the
  shipped SLUS load image, populated at runtime. Recover consumers/producers
  from code/xrefs; zero-filled EXE bytes are not evidence of absence.
- A callback table may intentionally target the concurrently loaded companion
  overlay (`GAME.EMI#0` tables mix `0x8019...` with `0x801d...`/`0x801e...`).
  Preserve the pointer as a reviewed table entry; do not create a local
  boundary because the target lies outside the payload map.
## Executable metadata

- Read PS-X EXE `t_addr` from header offset `0x18`; never assume the common
  `0x80010000` base. `SLUS_004.22` loads at `0x80096800`; `LOGO.EXE` at
  `0x801ce000`. A wrong base maps valid runtime addresses into unrelated zero
  padding and can make a library look all-zero or runtime-generated.
  Cross-check the manifest against the normalized binary's metadata and the
  original header, independently per executable.
## Build and matching

### Diagnose toolchain failures before changing candidate C

- If `bin/asm-diff TARGET@0xADDRESS` cannot compile a new lift, diff one
  known existing function from the same target; the same failure on both means
  a workspace/toolchain problem, not wrong candidate C.
- A compiler exit without diagnostics is not a comparison result. Preserve the
  last verified diff; fix the compile path before tuning source shape.
- The historical compiler is a statically linked 32-bit i386 executable; under
  a managed sandbox it can exit `225`/`159` before processing arguments.
  Re-run `bin/cc` with its approved out-of-sandbox permission; never add
  flags or rewrite C to address the exit.

### Preserve fixed-RAM pointer ownership before permuting source shape

- When m2c exposes a fixed-RAM address as a pointer-valued `D_XXXXXXXX`
  global, add the raw symbol to the target-local map and declare its narrowest
  evidence-backed type in `internal.h`; never hide a known RAM global behind
  an anonymous address macro.
- Add `volatile` only with asynchronous/hardware-mutation evidence
  (`func_800B2218` matched only after `D_80148648` became a named
  `PanelTask*`). Scheduling symptoms and levers:
  [playbook §Volatility](matching-playbook.md#volatility).
- Recover stable field offsets into a target-local struct before permuting.
  Addresses, masks, encoded values stay hexadecimal; human quantities
  (32-pixel step, 320-pixel clamp) decimal.
- Model stack locals with official PsyQ SDK types (e.g. `MATRIX`) when the
  evidence fits; ad-hoc `u32` arrays round to 8-byte slots and inflate the
  frame.
### Share duplicate behavior, not target ownership

- Exact bytes make a strong source-shape reuse candidate; they do not make
  one function address, extern declaration, or semantic provenance global.
  "Has authored C" ≠ "has a matching lift": percentage ranks effort, never
  authorizes propagation.
- Validate a second member independently before sharing code; normalize
  equivalent variables and struct fields before extracting a shared body.
- Use a semantic `src/shared/<domain>/*.inc` body only after two
  independently matching cross-target members; keep address-based wrappers so
  each target compiles/validates its own symbol.

### Reach fixed RAM through `PSX_PTR`/`PSX_REF`, never raw casts or `vu8`

- All fixed-address access goes through `include/memory/` and
  `include/base/`. The `vu8`/`vu16`/`vu32` typedefs are gone; write
  `volatile u8`/`u16`/`u32` directly on the `type` argument
  (`PSX_REF(volatile u16, 0x80143B90u)`).
- Scratchpad RAM (`0x1F800000`): `SPAD_ADDR`/`SPAD_REF`/`SPAD_PTR_SLOT`. Keep
  the slot cell non-volatile — a `volatile` slot forces `lui + ori + lw`;
  for a per-evaluation reload use `PSX_REF(Entity * volatile,
  SPAD_ADDRESS(off))`. See the [memory API](memory-api.md).

## Target ownership and symbols

- A shared-map (`config/targets/shared/symbols.txt`) `D_*` entry claims data
  at that vram in EVERY target; keep it in the owning target's local map
  unless it is data everywhere, else bogus contains-data functions appear.
- Exclude a data blob from Rizin's function list with `Cd <size> @ <addr>`
  in the target's `reviewed.rz`; `af-` does not survive the replay (`aa`
  re-creates it).

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
  semantic name; no compatibility aliases. Function names: verb-led
  camelCase, role-first (`dispatchScenarioState`), no module/target prefix
  except to break a collision. Data: camelCase + role suffix
  (`...Table`/`...Strings`/`...State`) + `// @source 0xXXXXXXXX` and
  `// @kind table|rodata|bss|data` tags (raw `D_*`: `@kind unknown`).
- Preserve pre-promotion evidence with an `INFERRED:` comment beside the
  owning address-based declaration: what was observed, what would verify
  promotion. Never create a semantic alias from a hint alone.
- Compare target-qualified analyzer snapshots under
  `out/reverse/<target>/snapshot.json`; equal addresses across targets are
  insufficient. Overlays and PsyQ copies can share a role with different
  addresses or bytes.
