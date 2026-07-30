---
name: bof3-re
description: Execute BOF3 target-qualified function lifting, exact matching, duplicate-group normalization, and evidence-gated source promotion. Use when the user invokes `$bof3-re` to select or lift BOF3 functions, update reviewed target source, Splat layouts or maps, or promote proven cross-target duplicate bodies.
---

# BOF3 Reverse Engineering

Operate on one independently loaded BOF3 target and function at a time.

## Load docs before editing

Read these before any lift. Do not skip to C edits.

**Always (every lift):**

1. `AGENTS.md` — boundaries, ownership, verification rules.
2. `docs/index.md` — documentation map (start here to find any spec).
3. `docs/usage.md` — tool commands and output budgets.
4. `docs/matching.md` — the iteration loop, header barrel, data rules.
5. `docs/matching-playbook.md` — symptom-to-lever table, all 18 sections.
6. `docs/memory-api.md` — PSX_PTR/PSX_REF/SPAD macros, inline-asm ban.
7. `LESSONS.md` — worked examples of past gotchas.

**When the function touches a domain (read the matching spec):**

- Runtime layout, overlays, load addresses → `docs/specs/runtime/*.md`
- EMI format, types, loader dispatch → `docs/specs/formats/emi.md`
- Graphics, palettes, VRAM → `docs/specs/formats/graphics.md`
- Data tables, structs, IDs → `docs/specs/data/*.md`
- Target map, load addresses → `docs/specs/targets.md`
- Verified algorithms → `docs/specs/pseudocode.md`
- Data discovery and verification → `docs/specs/methods.md`

**When doing analyzer or disassembly work, read psx-rizin references:**

- Full RE workflow phases → `.agents/skills/psx-rizin/references/WORKFLOW.md`
- Matching decompilation and build/diff → `.agents/skills/psx-rizin/references/DECOMP_BUILD_DIFF.md`
- Rizin commands and staged analysis → `.agents/skills/psx-rizin/references/RIZIN_PLAYBOOK.md`
- PSX ABI, addressing, delay slots → `.agents/skills/psx-rizin/references/PSX_ABI_AND_ADDRESSING.md`
- Symbols, signatures, types → `.agents/skills/psx-rizin/references/SYMBOLS_SIGNATURES_AND_TYPES.md`
- Overlays and assets → `.agents/skills/psx-rizin/references/OVERLAYS_AND_ASSETS.md`

Prefer the repo `bin/` entrypoints over generic psx-rizin scripts.

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
- Follow the naming convention in `docs/matching.md` §Naming convention:
  PascalCase structs, snake_case members, `(1 << N)` bitflags, `g_` globals,
  hex for addresses/offsets, decimal for human quantities.
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
3. **Check existing declarations before creating new ones** (see §Check
   existing below). Reuse what already exists; do not duplicate.
4. Run `bin/splat TARGET`, `bin/m2ctx TARGET@0xADDRESS`, and
   `bin/m2c TARGET@0xADDRESS -o out/candidate.c` as needed. The m2c seed emits
   only stub signatures (`extern void func_…()`); recover real types from
   callees/callers — never trust the seed's signatures.
5. Name SDK calls correctly: before writing `func_8017XXXX`, check the SDK map
   (`config/sdk/psyq-<space>.txt`) or `bin/symbols psyq-report TARGET`. If it is
   a known PsyQ/BIOS symbol, use the official name and header declaration; never
   lift its body. After any SDK-map edit, regenerate bindings with
   `bin/symbols psyq-bindings TARGET --write`.
6. Edit the address-owned C file and only evidence-required local header, map,
   or Splat entries.
7. Enter the **iteration loop** (below).
8. Accept only `bin/byte-match TARGET@0xADDRESS` equality (exit 0).

### Check existing

Before declaring any new function, struct, typedef, extern, `#define`, or
symbol, search for what already exists. Duplicate declarations cause conflicts,
divergent types, and wasted effort.

**Search order (fastest first):**

1. **Target `internal.h`** — grep the target's `src/<target>/internal.h` for
   the address, name, or a similar type/struct. If a struct with the same
   offsets exists, reuse it; extend it only with new evidence-backed fields.
2. **Target `symbols.txt`** — grep `config/targets/<target>/symbols.txt` for
   the address or name. If the symbol is already mapped, use the existing name.
3. **Shared headers** — grep `include/bof3/` for shared types, macros, and
   constants (`core.h`, `bof3.h`, `defines.h`, `memory.h`, `scratchpad.h`,
   `psyq.h`). Do not redeclare what `include/bof3/` already provides.
4. **SDK maps** — `bin/symbols psyq-report TARGET` or grep
   `config/sdk/psyq-<space>.txt` for PsyQ/BIOS symbols. Use official names.
5. **Cross-target index** — `bin/rev-query symbols <name-or-address>` and
   `bin/rev-query variables <name-or-address>` to find symbols declared in
   other targets. Do not copy an extern address across targets (each target
   binds its own), but do reuse the same struct layout or naming convention
   when the evidence supports it.
6. **Sibling targets** — for functions in the same family (e.g. other
   `world00/area*` targets), check their `internal.h` for similar structs
   or patterns. Duplicate groups often share the same struct layout.

**Rules:**

- If a symbol for the address already exists in the target map, use that name.
  Do not create a second name for the same address.
- If a struct with matching offsets exists, extend it rather than creating a
  parallel struct with a different name.
- If a `#define` or constant exists in `include/bof3/`, use it. Do not
  redefine it locally.
- If a PsyQ symbol is known, use the official declaration from the SDK headers.
  Do not write a local prototype for a known SDK function.
- When adding a new extern to `internal.h`, verify the address is not already
  declared under a different name in the same file.

## Iteration loop

This is the core discipline. The most common failure mode is making many tiny
edits without ever reading the assembly diff. This loop prevents that.

### HARD GATE: no C edits without asm-diff

**Do NOT edit C until you have run `bin/asm-diff` and read its output.**
This applies before the first edit and before every subsequent edit. No
exceptions. If you cannot run asm-diff (compile error, toolchain issue),
diagnose the build problem first (`LESSONS.md` §Diagnose toolchain failures)
— do not guess at C changes.

### Step A — Run asm-diff and read the assembly

```sh
bin/asm-diff TARGET@0xADDRESS --detail full
```

Then **read the full diff** under `out/asm-diff/`. Not just the summary line.
The full file shows original-side and current-side instructions side by side.

From the diff, determine:

1. **Where**: the first mismatch at `first=+0xOFF[idx]`.
2. **What the original does**: the exact instruction, its operands, load/store
   width, branch direction, delay slot content.
3. **What the current side does**: the corresponding instruction and how it
   differs.
4. **Why**: classify the root cause:

| Category | Signal in the diff | Lever |
|---|---|---|
| Declaration/type | wrong load/store width (`lb` vs `lbu`), `lw` where address calc expected, wrong relocation symbol | Fix the C type or declaration |
| Control-flow shape | `beq` vs `bne`, wrong loop topology, wrong `$v0` return web | Restructure if/else/loop/return |
| Expression ordering | right instructions in wrong order, stores before loads | Reorder statements, add temps |
| Register allocation | right instructions, wrong register names | Temp hoist, statement reorder, induction variable |
| Compiler profile | systematic width/sign differences, extra `andi`, wrong shift pattern | `bin/flag-search` |
| Data/symbol | wrong global offset, phantom `.rodata`, BSS order, `$gp` reach | Fix symbol, section, data declaration |

### Step B — One diagnosed fix

Apply **one** fix that addresses the classified root cause at `first=`.

An "attempt" is a **diagnosed structural change**, not a tiny tweak. Examples
of valid attempts:

- Changing a struct field from `u8` to `u16` because the original uses `lhu`.
- Inverting an `if/else` because the original has `bne` where you have `beq`.
- Converting a `while` loop to `do/while` because the original tests at the bottom.
- Hoisting a pointer dereference because the original reuses a base register.

Examples of **invalid** attempts (do not count toward the retry budget):

- Adding a cast without understanding why the width is wrong.
- Renaming a variable.
- Adding a comment.
- Reordering two independent statements without reading the register diff.

After the fix:

```sh
bin/asm-diff TARGET@0xADDRESS --detail normal
```

### Step C — Verify the result

After every edit, check three things:

1. **Did `first=` advance?** (the first mismatch moved to a later offset)
2. **Did the percentage improve or stay equal?**
3. **Are there new mismatches that were hidden before?**

**ABSOLUTE REVERT RULE**: if the match percentage dropped, **revert the
change immediately** before trying anything else. Do not build on a
regression. Do not assume a later fix will compensate. Revert first, then
re-diagnose from the previous state.

### Step D — Bounded retry with strict escalation

Track attempts per level. After **3 diagnosed attempts** at the same level
with no progress on `first=`, escalate to the next level. **Escalation is
strictly ordered** — always start at Level 1 and work up. Do not skip levels
even if the diagnosis seems obvious; type/declaration bugs are the most common
root cause and are easily missed.

```text
Level 1 — Types and declarations (3 attempts max):
  pointer vs array, standalone symbol vs struct field, signedness,
  access width (u8/u16/u32/s8/s16/s32), volatile qualification,
  enum vs int, function signature return/param types.
  → no progress on first= after 3 attempts → Level 2.

Level 2 — Control-flow shape (3 attempts max):
  invert if/else (beq↔bne), change loop shape (while/do/for/goto),
  early return vs result variable, reorder independent blocks,
  switch lowering form, guarded infinite loop.
  → no progress on first= after 3 attempts → Level 3.

Level 3 — Expression and register ordering (3 attempts max):
  pointer hoist, expression split/collapse, named constant reuse,
  induction variable form (counter vs pointer), statement reorder,
  barrier()/CLOBBER_* for delay-slot placement, dead-code preservation
  (matching-playbook §6).
  → no progress on first= after 3 attempts → Level 4.

Level 4 — Compiler profile (3 attempts max):
  bin/flag-search TARGET@0xADDRESS
  Test non-canonical -O level, -G value, signed-char, split-addresses.
  If a profile byte-matches clean C, record it in
  config/compiler/object-flags.cmake.
  → no progress after 3 attempts → Level 5.

Level 5 — Permuter (1 bounded run):
  Prerequisite: structure and declarations must already be correct.
  bin/permute TARGET@0xADDRESS --time-limit 300 -j N
  Inspect winning mutations, simplify the winning source, re-run to
  remove unnecessary hacks. The permuter is a search tool, not a
  structural fix — it cannot repair wrong types or wrong control flow.
  → no match after 1 run → Level 6.

Level 6 — One local pin experiment or documented residual:
  For an asm-diff-proven allocator or entry-register residual, make one bounded
  local `REGISTER_PIN(type, name, reg)` experiment with a `MATCHING_AID`
  rationale. Retain it only after a live exact byte match and independent review.
  Otherwise report the residual. Direct numeric pins and INCLUDE_ASM still need
  explicit user approval.
```

**Escalation is additive, not destructive.** Each level builds on the
previous. If Level 2 fixes control flow but register issues remain, go back
to Level 3 levers for the remaining diff. If a Level 1 issue is discovered
at Level 3, fix it at Level 1 and re-check.

### Anti-patterns

These are the observed failure modes. Do not do them:

- **Editing C without running asm-diff first.** This is the #1 failure.
  You cannot fix what you have not measured.
- **Making 5+ edits without reading the diff between them.** Each edit
  must be preceded by reading the current diff output.
- **Running the permuter before fixing types and control flow.** The
  permuter searches source shapes; it cannot repair a wrong struct layout
  or an inverted branch.
- **Changing multiple unrelated things in one edit.** You cannot diagnose
  which change helped or hurt.
- **Ignoring `first=` and fixing a later mismatch.** Always converge from
  the first mismatch forward.
- **Trusting m2c output without checking against the original asm.** m2c
  is a hypothesis generator, not ground truth.
- **Adding `volatile`, `barrier()`, or temps speculatively** without
  understanding why the original has a specific instruction order.
- **Treating a percentage increase as success.** Only `bin/byte-match`
  exit 0 is success.
- **Building on a regression.** If the percentage dropped, revert first.

### Reading `asm-diff` output

`bin/asm-diff` prints `MATCH|DIFF <fn>@<addr> insn=O/C(N%) bytes=O->C(+D)
first=+0xOFF[idx] diff=…` and exits 0 only on an exact byte match. `first=` is
the offset (and instruction index) of the first mismatch — start there.
`--detail normal` shows only the first hunk (≤24 lines); the full
original-vs-current diff is under `out/asm-diff/`. Treat the original side as
ground truth and converge the current side onto it.

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
8. For an asm-diff-proven allocator or entry-register residual, make one bounded
   local `REGISTER_PIN(type, name, reg)` experiment with `MATCHING_AID`; retain
   only after a live exact byte match and independent review. Direct numeric pins
   and `INCLUDE_ASM` still require explicit user approval.

Document every artificial matching aid with a `MATCHING_AID` comment (see
`docs/matching-playbook.md` §4). Do not add generic macros to headers for
matching hacks.

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
- No inline `__asm__` of any kind in lifted source — that includes direct
  `register X asm("$N")` register pins and `extern X asm("NAME")` symbol renames,
  both of which need explicit user approval. After the matching ladder, the only
  allowed allocator constraint is one local `REGISTER_PIN(type, name, reg)`
  experiment for an asm-diff-proven allocator or entry-register residual; retain
  it only with `MATCHING_AID`, independent review, and a live exact byte match.
  The other sanctioned helpers are `barrier()` (access ordering) and
  `CLOBBER_A0()/CLOBBER_V0()/CLOBBER_A1()`
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
