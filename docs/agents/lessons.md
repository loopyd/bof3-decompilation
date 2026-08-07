# Reverse-engineering lessons

Domain contracts belong in `docs/specs/`; repeatable procedures in the owning
operating reference. Matching levers: [matching playbook](matching-playbook.md);
iteration procedure: [function matching](matching.md).

## Evidence and boundaries

### Validate payload boundaries against bytes

- Splat labels are reviewed inputs, not stronger evidence than payload bytes;
  a plausible decoded instruction can still be embedded data
  (`emi/world00/area008/13`: `"%d"` header at payload `0x14`, code at
  `0x18`). Check calls, saved return addresses, and return paths; split
  confirmed leading data in Splat before promoting the real boundary.
- Raw EMI payloads may carry a header before the code VRAM; reconcile
  analyzer addresses with payload offsets before adding boundaries
  (`runtime address - load address == payload offset`). Never load an
  extracted `GAME.EMI` entry linearly at its first function address: entry 0
  opens with a count/pointer header, entry 1's title setup handler sits at
  payload `0x90`; `GAME.EMI#0` (load `0x80195800`) starts code at payload
  `0x91c`. A bad base silently shifts every byte read, and sequence-based asm
  resolution can still report exact matches under it.
### Account for runtime state and cross-target pointers

- Frontend callback tables at `0x801c7b08`/`0x801c7b14` are zero in the
  shipped SLUS image, populated at runtime; recover consumers/producers from
  code/xrefs — zero-filled bytes are not evidence of absence. A table may
  intentionally target the concurrently loaded companion overlay (`GAME.EMI#0`
  tables mix `0x8019...` with `0x801d...`/`0x801e...`): preserve the pointer
  as a reviewed entry; never create a local boundary for an address outside
  the payload map.
## Executable metadata

- Read PS-X EXE `t_addr` from header offset `0x18`; never assume the common
  `0x80010000` base (`SLUS_004.22` loads at `0x80096800`; `LOGO.EXE` at
  `0x801ce000`). A wrong base maps valid runtime addresses into unrelated
  zero padding and can make a library look all-zero or runtime-generated.
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
  global, map the raw symbol target-locally and declare its narrowest
  evidence-backed type in `internal.h`; never hide a known RAM global behind
  an anonymous address macro.
- Add `volatile` only with asynchronous/hardware-mutation evidence
  (`func_800B2218` matched only after `D_80148648` became a named
  `PanelTask*`); levers: [playbook §Volatility](matching-playbook.md#volatility).
- Tail-dispatch prologue between index load and `sll`: a `const` table
  extern can block that schedule; dropping `const` is a lever alongside the
  local-copy + `barrier()` shape. Try both before pins.
- Recover stable field offsets into a target-local struct before permuting.
  Addresses, masks, encoded values stay hexadecimal; human quantities
  (32-pixel step, 320-pixel clamp) decimal.
- C alignment silently relocates a misdeclared field: a `u32` at an
  unaligned offset (e.g. `unk_49`) lands on the next boundary and shifts
  later fields. Model unaligned words as `u8[N]`; audit offsets against the
  struct's real C layout, not the comments.
- Model stack locals with official PsyQ SDK types (e.g. `MATRIX`) when the
  evidence fits; ad-hoc `u32` arrays round to 8-byte slots and inflate the
  frame.
### Share duplicate behavior, not target ownership

- Exact bytes make a strong source-shape reuse candidate; they globalize no
  address, extern, or provenance. "Has authored C" ≠ "has a matching lift":
  percentage ranks effort, never authorizes propagation.
- Validate a second member independently before sharing code; normalize
  equivalent variables and struct fields before extracting a shared body.
- Use a semantic `src/shared/<domain>/*.inc` body only after two
  independently matching cross-target members; keep address-based wrappers so
  each target compiles/validates its own symbol.

### Argument-register pins can be a legitimate allocator residual

- A retained `REGISTER_PIN` targeting an `a*` register is not automatically an
  entry-copy problem: when the original frees the argument early (entry copy
  into a callee-saved register in the prologue) and reuses the freed `a*`
  register as a scratch load destination (`lhu a1` / `andi v0,a1,...`), that
  is an allocator residual. The same ladder, `MATCHING_AID`, live byte-match,
  and independent-review requirements apply; the split load/mask register
  pair may need one pin per register.

### Reach fixed RAM through `PSX_PTR`/`PSX_REF`, never raw casts or `vu8`

- All fixed-address access goes through `include/memory/` and
  `include/base/`; write `volatile u8`/`u16`/`u32` directly on the `type`
  argument (`PSX_REF(volatile u16, 0x80143B90u)`).
- Scratchpad RAM (`0x1F800000`): `SPAD_ADDR`/`SPAD_REF`/`SPAD_PTR_SLOT`. Keep
  the slot cell non-volatile (a `volatile` slot forces `lui + ori + lw`); for
  a per-evaluation reload use `PSX_REF(Entity * volatile,
  SPAD_ADDRESS(off))`. See the [memory API](memory-api.md).

## Target ownership and symbols

- A shared-map (`config/targets/shared/symbols.txt`) `D_*` entry claims data
  at that vram in EVERY target; keep it in the owning target's local map
  unless it is data everywhere — else bogus contains-data functions appear.
- Exclude a data blob from Rizin's function list with `Cd <size> @ <addr>`
  in the target's `reviewed.rz`; `af-` does not survive the replay (`aa`
  re-creates it).
- Check the shared SDK maps (`config/sdk/psyq-*.txt`) before adding to a
  target-local map: Splat composes both and a duplicate name aborts
  `bin/splat`. Keep each symbol in exactly one map.
- PsyQ code can be linked at different addresses across executables and
  EMI payloads; an address verified in `SLUS_004.22` is not a contract for
  another binary.
- Use official PsyQ names; record the verified archive member in the owning
  target's map. Generated weak bindings take the runtime address from it.
- Replace analyzer aliases only after behavior and signature are proven;
  raw `func_XXXXXXXX`/`D_XXXXXXXX` names go straight to the reviewed
  semantic name, no compatibility aliases. Functions: verb-led camelCase,
  role-first, no target prefix except to break a collision. Never prefix a
  raw `D_*`/`func_*` name with an overlay name (`SCENA16_D_*`); a collision
  resolves by a different name or a suffix (`D_80146864_BYTE`). Data:
  camelCase + role suffix (`...Table`/`...Strings`/`...State`) +
  `/* @source 0xXXXXXXXX` and `@kind table|rodata|bss|data */` tags (raw
  `D_*`: `@kind unknown`; `/* */` only — `//` breaks gcc-2.6.3). Every
  non-address-named map symbol (SDK exempt) needs one @source-tagged
  definition: its lift file, a header/source declaration, or a
  `WEAK_SYMBOL_AT` binding; `bin/symbols check` enforces both rules.
- Preserve pre-promotion evidence with an `INFERRED:` comment beside the
  owning address-based declaration: what was observed, what would verify
  promotion. Never create a semantic alias from a hint alone.
- Compare target-qualified snapshots (`out/reverse/<target>/snapshot.json`);
  equal addresses across targets are insufficient — overlays and PsyQ copies
  can share a role with different addresses or bytes.
- Every `WEAK_SYMBOL_AT` in the hand-maintained top-level
  `src/<target>/symbols.c` needs a target-map entry; a different name at a
  mapped address is a deliberate typed alias (e.g. u8 view of a u16 global).
  `bin/symbols check` flags bindings whose address no map owns.
