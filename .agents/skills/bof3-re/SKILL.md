---
name: bof3-re
description: Execute BOF3 target-qualified function lifting, exact matching, duplicate-group normalization, and evidence-gated source promotion. Use when the user invokes `$bof3-re` to select or lift BOF3 functions, update reviewed target source, Splat layouts or maps, or promote proven cross-target duplicate bodies.
---

# BOF3 Reverse Engineering

Operate on one independently loaded BOF3 target and function at a time. Read
the repository `AGENTS.md`, then read `docs/usage.md`; read `docs/matching.md`
for the iteration loop; consult `docs/matching-playbook.md` when resolving
specific asm-diff symptoms (register swaps, branch direction, control-flow
shape, temporaries, pinning).

## Rules

- Qualify identity as `TARGET@0xADDRESS`; original bytes and PS-X headers win.
- Keep raw address-based filenames and target-local maps, declarations, and
  Splat ranges.
- Treat m2c, Rizin, signatures, rankings, and partial matches as hypotheses.
- Write readable C89; never use handwritten assembly to force a match.
- Keep each target `internal.h` a structured barrel in this order: include
  guard → `#include`s → types (`typedef`/`struct`/`enum`) → external variables
  (`extern`) → external function prototypes → `#define` macros and `static
  inline` helpers at the bottom. A `typedef` used by an `extern` precedes it; a
  `static inline` that uses a `#define` follows that macro. See
  `docs/matching.md` §Header barrel convention.
- Add no tests for lifted game behavior. Add only the least tooling-contract
  test when tooling changes require one.
- Never commit unless the current user explicitly requests a commit.

## Choose scope

- Obey an explicit function or duplicate-group choice.
- When asked for guidance, rank candidates with `bin/rev-query` (`quick-wins`,
  `leafs`, `duplicates`, `hotspots`, `pareto`) using `--unlifted --detail
  minimal --limit 5`, and report effort (`instruction_count`,
  `cyclomatic_complexity`), callers/callees, `duplicate_leverage`, and
  confidence (`confidence_band`/`metric_missing`).
- When the user has not chosen, recommend but wait for their selection before
  editing. Do not silently choose a different scope.

## Lift

1. Validate the manifest, reviewed boundary, and target map. Confirm the load
   address: `runtime_address − load_address == payload_offset` (read `t_addr`
   from the PS-X header at offset `0x18`). A green diff does NOT validate a
   wrong load address (`LESSONS.md`).
2. Gather evidence with `bin/rev-query calls TARGET@0xADDRESS` and
   `bin/rev-query duplicates TARGET@0xADDRESS`: callees supply prototypes to
   declare, callers fix the signature/ABI, a duplicate group seeds the shape,
   and `unresolved_calls`/`metric_missing` flag risk.
3. Run `bin/splat TARGET`, `bin/m2ctx TARGET@0xADDRESS`, and
   `bin/m2c TARGET@0xADDRESS -o out/candidate.c` as needed. The m2c seed emits
   only stub signatures (`extern void func_…()`); recover real types from
   callees/callers — never trust the seed's signatures.
4. Name SDK calls correctly: before writing `func_8017XXXX`, check the SDK map
   (`config/sdk/psyq-<space>.txt`) or `bin/symbols psyq-report TARGET`. If it is
   a known PsyQ/BIOS symbol, use the official name and header declaration; never
   lift its body. After any SDK-map edit, regenerate bindings with
   `bin/symbols psyq-bindings TARGET --write`.
5. Edit the address-owned C file and only evidence-required local header, map,
   or Splat entries.
6. Iterate with `bin/asm-diff TARGET@0xADDRESS --detail normal`.
7. Accept only `bin/byte-match TARGET@0xADDRESS` equality (exit 0).

### Resolving asm-diff mismatches

When the semantic C is correct but instruction bytes differ, consult
`docs/matching-playbook.md` §Symptom-to-lever-table. Common fixes in order:

1. Fix types and declarations (pointer vs array, standalone symbol vs struct field).
2. Invert `if/else` for branch direction.
3. Change loop shape (`while`, `do`, `for`, `goto`).
4. Use early returns instead of result variables.
5. Hoist pointer dereferences; introduce or remove temporaries.
6. Check compiler profile (`bin/flag-search`) and signedness; if a non-canonical
   profile byte-matches clean C, record it in `config/compiler/object-flags.cmake`.
7. Run the permuter as a bounded search, not a structural fix.
8. Register pinning (`register X asm("$N")`) and `INCLUDE_ASM` are banned unless
   the user explicitly approves them; report a documented residual instead.

Document every artificial matching aid with a `MATCHING_AID` comment (see
`docs/matching-playbook.md` §4). Do not add generic macros to headers for
matching hacks.

### Reading `asm-diff` output

`bin/asm-diff` prints `MATCH|DIFF <fn>@<addr> insn=O/C(N%) bytes=O->C(+D)
first=+0xOFF[idx] diff=…` and exits 0 only on an exact byte match. `first=` is
the offset (and instruction index) of the first mismatch — start there.
`--detail normal` shows only the first hunk (≤24 lines); the full
original-vs-current diff is under `out/asm-diff/`. Treat the original side as
ground truth and converge the current side onto it.

### Recovering structs and data

Infer struct layout from consumers before naming: collect the offsets each
caller/callee accesses, name unknown fields `unk_XX` by byte offset, pin the
layout with `ASSERT_OFFSET`/`ASSERT_SIZE`, then promote `unk_XX` to
evidence-backed names (`docs/matching-playbook.md` §2/§10,
`docs/specs/methods.md`). Define target-owned data (zero-init BSS included);
keep data you do not own `extern` (`docs/matching.md`).

## Address and scratchpad access

Use the single source of truth in `include/bof3/`; the authoritative standard
is `docs/matching.md` (matching loop and header convention), and the full
macro reference is in `docs/memory-api.md`.

- `PSX_PTR(type, addr)` / `PSX_REF(type, addr)` — fixed-address pointer / lvalue.
  Qualify with `const`/`volatile` on the type: `PSX_REF(volatile u16, 0x80143B90u)`.
- `REG8/16/32(addr)` — hardware registers only (never scratchpad RAM).
- `FIELD_ADDR`/`FIELD_REF(type, base, byte_offset)` — incomplete-struct offset access.
- `SPAD_ADDR`/`SPAD_REF(type, byte_offset)` and `SPAD_PTR_SLOT(type, byte_offset)`
  for scratchpad RAM (`0x1F800000`–`0x1F8003FF`).

Rules:

- No `vu8`/`vu16`/`vu32` aliases — write `volatile u8` etc. directly.
- No inline `__asm__` of any kind in lifted source — that includes
  `register X asm("$N")` register pins and `extern X asm("NAME")` symbol renames,
  both of which need explicit user approval. The only sanctioned helpers are
  `barrier()` (access ordering) and `CLOBBER_A0()/CLOBBER_V0()/CLOBBER_A1()`
  (delay-slot register placement) from `include/bof3/defines.h`. Bind
  fixed-address symbols with a plain `extern` in `internal.h` plus
  `WEAK_SYMBOL_AT(name, addr)` in the target `symbols.c` (see
  `docs/memory-api.md`). See `LESSONS.md` for worked examples.
- Pointer cells: `PSX_REF(type *, addr)` (non-volatile) vs
  `PSX_REF(type * volatile, addr)` (volatile cell, reloaded each evaluation).
  `SPAD_PTR_SLOT` is intentionally non-volatile — the constant-address codegen
  (`lui` + offset load) matches the original binary. Marking the cell volatile
  forces `lui + ori + lw` and breaks the match (`func_801B5BDC`). Qualify the
  pointee, not the cell, to force a reload.

## Promote duplicates

1. Confirm identical reviewed bytes and boundaries.
2. Make one representative byte-match.
3. Make a cross-target second member byte-match independently.
4. Normalize only evidence-backed roles, parameters, locals, types, fields,
   and constants; retain raw filenames.
5. Keep small or same-target pairs separate unless the user chooses otherwise.
6. When reuse is worthwhile, move only the embedded body to
   `src/shared/<domain>/<role>.inc`; keep one target wrapper per raw function.
7. Put stable shared types in `include/bof3/<domain>/`.

A shared template compiles into each owning EMI code blob. It is not a runtime
engine service. A real service has one implementation under
`src/exe/slus_004_22/` plus EMI callsite evidence; promote only its proven
contract to `include/bof3/core.h`. Never cross-link EMIs or invent `src/engine/`
ownership.

## Verify and hand off

- Recheck every promoted member with `asm-diff` and `byte-match`.
- Run `bin/symbols check`, relevant Splat/build/status checks, and
  `git diff --check`; keep full evidence in `out/`.
- Report `Done`, `Evidence`, `Checks`, `Skipped`, and `Next` concisely.
- Suggest optional semantic or structural improvements for user choice.
