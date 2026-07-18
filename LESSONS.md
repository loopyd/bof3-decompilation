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

### Force global read/store order with a local when m2c reorders

- m2c commonly reorders independent global accesses. A source like
  `flag = 2; counter += 0x14;` may emit the constant-load and `sb` store before
  the `lhu` read of `counter`, breaking byte-match even though the semantics
  match.
- Introduce a local to pin the original load-then-store order:
  `count = counter; flag = 2; counter = (u16)(count + 0x14);` reproduces the
  original read-before-store stream. Keep the narrow-width cast so the store
  width matches. This byte-matched `func_801F4578`/`func_801F3258`
  (0x8014932A/0x80149333 pair) across `emi/world00/area008/13` and
  `emi/world00/area026/13`.

### Force global-read ordering with `barrier()` when the psn00b scheduler diverges

- PsyQ GCC 2.7.2 schedules `addiu sp,sp,-24` / `sw ra,16(sp)` **after** a global
  `lhu` read but before an indirect call. psn00b GCC 12.3.0 moves the prologue
  before the read, producing a 3-instruction offset (80% match).
- `__asm__ __volatile__("" : : : "memory")` emits zero instructions and only
  prevents compiler reordering across the barrier. This is the **standard
  practice** across PS1 decomp (SOTN, Frogger, Skullmonkeys) and the Linux
  kernel (`barrier()` macro in `include/linux/compiler-gcc.h`).
- `volatile` on the variable or a volatile handler-slot cast does **not** fix
  the scheduling — tested at 66.67% match. `-fno-schedule-insns2` has no effect
  on psn00b GCC.
- Sources:
  - [GCC internals — `"memory"` clobber](https://www.chiark.greenend.org.uk/doc/gcc-4.3-doc/gccint.html)
  - [Stack Overflow — `asm volatile("": : :"memory")` vs `__sync_synchronize`](https://stackoverflow.com/questions/19965076)
  - [Decompedia — PS1 platform](https://decomp.wiki/platforms/playstation)

### Preserve fixed-RAM pointer ownership before permuting source shape

- When m2c exposes a fixed-RAM address as a pointer-valued `D_XXXXXXXX`
  global, add that raw symbol to the target-local map and declare its narrowest
  evidence-backed type in the target `internal.h`. Do not hide a known RAM
  global behind an anonymous address macro.
- Match qualifiers to the observed contract. An unjustified `volatile`
  pointee can change register allocation and move stores across comparisons;
  `func_800B2218` matched only after `D_80148648` became a named
  `Bof3PanelTask*`. Add `volatile` only when asynchronous or hardware
  mutation is part of the evidence.
- Recover stable field offsets into a target-local struct before trying
  permutations. Keep addresses, masks, and encoded values hexadecimal; write
  human quantities such as the 32-pixel step and 320-pixel clamp in decimal.

### Share duplicate behavior, not target ownership

- Exact bytes make a strong source-shape reuse candidate. They do not make one
  function address, extern declaration, or semantic provenance global.
- "Has authored C" is not the same as "has a matching lift." This run rejected
  a 7.95% representative despite exact group bytes; match percentage ranks
  effort but does not authorize propagation.
- Validate a second member independently before sharing code so target-local
  compiler context cannot become hidden plumbing.
- Normalize equivalent variables and struct fields before extracting a shared
  body. A shared implementation with divergent local vocabulary only hides
  unresolved understanding.
- Use a semantic `src/shared/<domain>/*.inc` body after two independently
  matching cross-target members when the reuse offsets the indirection.
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

### Document `MATCHING_AID` matching hacks

- Every artificial matching-aid comment must say exactly what it controls, why
  it is needed, and what future evidence would remove it. This ensures the hack
  is removable, not permanent.
- Do not mark obvious workarounds such as `barrier()`; reserve `MATCHING_AID`
  for shape decisions that are opaque to a reader without the matching diff.

```c
/*
 * MATCHING_AID:
 * Hoisting the slot pointer keeps the index temporary in $a1.
 * Without this local, GCC allocates $v1 for the index and the store
 * at +0x34 uses a different base.
 */
slots = *slotTable;
```

The full convention is documented in `docs/matching-playbook.md` §4.

### Use the register pinning ladder as a last resort

- `register type name asm("$N")` binds a local to a specific MIPS register but
  changes the entire register web. A pinned local remains live across the whole
  function and can displace unrelated variables, creating new mismatches.
- Use this escalation before pinning:
  1. Correct types and declarations.
  2. Correct control-flow structure.
  3. Reorder declarations and statements.
  4. Introduce or remove temporaries.
  5. Hoist pointer dereferences.
  6. Try a separate loop counter vs pointer induction variable.
  7. Run the permuter.
  8. Only then use `register ... asm("$N")`.
  9. Prefer to remove pins after discovering a structural solution.
- Never create a macro such as `FORCE_REG(type, name, reg)` — that would make
  register pinning too easy to spread.
- The `CLOBBER_A0()`/`CLOBBER_V0()`/`CLOBBER_A1()` macros in `defines.h`
  are empty-asm barriers for delay-slot ordering, not register pinning. They
  are lighter and safer when only placement matters.
- All inline `__asm__` must go through a named, `__GNUC__`-guarded macro in
  `defines.h` (`barrier()`, `CLOBBER_A0()`, `CLOBBER_V0()`, `CLOBBER_A1()`).
  Never write raw `__asm__ __volatile__(...)` in a `.c` file. The `CLOBBER_*`
  macros tell the compiler the named register is clobbered, which prevents it
  from hoisting a `move`/constant load out of a `jal`/branch delay slot.
  Example: `func_801970EC` needed `CLOBBER_A0();` before
  `func_801C1400(0u)` so `move a0,zero` stayed in the `jal` delay slot (100%).
- Rule of thumb: `barrier()` when the issue is volatile-access ordering across
  a call; `CLOBBER_A0()/V0()/A1()` when the issue is which register holds a
  value in a delay slot.

### Reach fixed RAM through `PSX_PTR`/`PSX_REF`, never raw casts or `vu8`

- All fixed-address access goes through the `include/bof3/` macros. The
  `vu8`/`vu16`/`vu32` typedefs are gone; write `volatile u8`/`u16`/`u32`
  directly on the `type` argument (`PSX_REF(volatile u16, 0x80143B90u)`).
- Hardware registers use `REG8/16/32` only. Scratchpad RAM (`0x1F800000`) uses
  `SPAD_ADDR`/`SPAD_REF`/`SPAD_PTR_SLOT` — never `REG*`. Full reference and the
  old→new migration table are in `docs/memory-api.md`.

### Keep `SPAD_PTR_SLOT` cell non-volatile to match constant-address codegen

- `SPAD_PTR_SLOT(type, off)` expands to a **non-volatile** pointer cell
  (`PSX_REF(type *, addr)`). The compiler then emits a single `lui` for the
  scratchpad base plus an offset `lw`/`lh`/`lb` (`lw v1,68(v1)` for
  `0x1F800044`), matching the original binary.
- Marking the slot `volatile` (e.g. `PSX_REF(type * volatile, addr)`) forces
  `lui + ori + lw`, adding an `ori` and breaking the match. `func_801B5BDC`
  dropped from 100% to a mismatch under the volatile slot and recovered only
  after the cell stayed non-volatile.
- To force a per-evaluation reload, qualify the pointee, not the cell:
  `SPAD_PTR_SLOT(volatile Entity, off)` (gives `volatile Entity **`) instead of
  a volatile cell.

### Hoist pointer dereferences to control register allocation

- Instead of repeated `(*table)[index]` accesses, hoist the dereference into a
  local: `entries = *table; value = entries[index];`. This can free the
  allocator to place an unrelated value in the required register and enable a
  clean, pin-free match.
- The full technique with expression splitting, named-constant reuse, and
  induction-variable alternatives is in `docs/matching-playbook.md` §5.

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
