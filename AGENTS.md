# AGENTS.md — BOF3

BOF3 binaries load independently. Qualify work by one `TARGET@0xADDRESS`.

## Boundaries

- Work only in this repository. Never commit user media from `inputs/`.
- `out/` is disposable working state: edit candidates there when a tool's
  workflow requires it, but never treat it as reviewed truth or commit it.
- Do not hand-edit `build/` or `toolchains/`.
- Analyze an executable image or extracted EMI entry, never an EMI archive.
- Keep identical addresses or bytes in different targets as separate facts.
- Original bytes and PS-X headers outrank analyzer output. Verify `t_addr`.

## Ownership

| Fact                             | Owner                          |
| -------------------------------- | ------------------------------ |
| Binary identity and load address | `config/targets/<target>.toml` |
| Reviewed layout                  | `config/splat/`                |
| Target-local symbols             | `config/symbols/<target>.txt`  |
| Reviewed Rizin annotations       | `config/analysis/<target>/`    |
| Authored lifts                   | `src/exe/`, `src/emi/`         |

## Source and symbols

- Keep one `func_XXXXXXXX.c` per function and an adjacent `internal.h`.
- Use `func_80143B40` and `D_80143B40`; maps use sorted
  `name = 0xADDRESS;` entries with eight uppercase hex digits.
- Replace raw names only after review. Keep lifted filenames address-based.
- Edit `func_XXXXXXXX.c`, its target `internal.h`, the target-local map, and
  reviewed Splat boundaries as evidence improves. Keep declarations local
  unless a demonstrated cross-target contract requires sharing.
- Keep PsyQ external: use official declarations and target-local map evidence;
  never lift its bodies. The PsyQ/BIOS runtime is a shared SDK linked once in
  the main exe; EMI overlays call those functions at the same fixed addresses,
  so their `psyq.c` bindings legitimately reuse the exe's PsyQ name/address set.
  This cross-target reuse is authorized by the pinned SDK version
  (`docs/reverse-engineering.md`), not by coinciding game bytes. Treat the
  extracted SDK symbols as a switchable weak-binding layer
  (`config/sdk/psyq-*.txt`, `WEAK_SYMBOL_AT`) that a real SDK library can later
  override one symbol at a time.
- Never edit or track generated weak bindings under `out/bindings/`.
- Write readable C89. Do not use handwritten assembly to force a match.

## Exact duplicates

- Treat `(analyzer-range SHA-256, size)` as a reuse candidate, not shared
  ownership. Confirm reviewed boundaries, then iterate on one representative.
- A tracked or partial lift is not a reusable implementation. Use its match
  percentage only to prioritize the next edit; do not promote other members
  from it.
- Port the representative C shape to a second member only after the first
  byte-matches. Keep both sources independent until both byte-match.
- Normalize proven roles, parameters, local names, struct names, and field
  names across group members. Keep unknown fields offset-based (`unk_XX`).
- After two cross-target members independently byte-match with the same C
  shape, move only a worthwhile common body to
  `src/shared/<domain>/<role>.inc`. Keep one
  address-based `func_XXXXXXXX.c` wrapper per member to provide its raw symbol
  and any compile-time parameters.
- Put stable shared types in `include/bof3/<domain>/`. A shared template is
  compiled into every owning image; it is not a runtime engine service.
- Never reuse a game-specific extern address across targets. Each wrapper
  retains its target-local map, declaration, Splat boundary, and independent
  `asm-diff`/`byte-match` validation. The shared PsyQ/BIOS runtime is exempt
  (see above).

## Verification

- Use `bin/asm-diff` for instruction evidence and `bin/byte-match` for bytes.
- Run `bin/symbols check` after map edits; normalize only with
  `bin/symbols normalize [TARGET] --write`.
- Run `bin/decomp-status [TARGET...]` for the live lift audit.
- Run `just check` before handoff when practical; state skipped checks.
- Do not stage, commit, push, or mutate external systems without approval.

Use `$bof3-re` for lifting and promotion, `$psx-rizin` for explicitly requested
generic analyzer work, and `$workflow-review` for explicitly requested
two-reviewer audits. Use [matching](docs/matching.md),
[foundation](docs/decomp-foundation.md), and
[Rizin evidence](docs/reverse-engineering.md) for procedures. See
[docs/memory-api.md](docs/memory-api.md) for the memory-macro reference. Store
reviewed findings in `docs/specs/` and
reusable evidence-backed gotchas in `LESSONS.md`. Use the
[repository map](CONTEXT.md#repository-map) to locate tracked and ignored state.
