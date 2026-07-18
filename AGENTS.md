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

| Fact | Owner |
| --- | --- |
| Binary identity and load address | `config/targets/<target>.toml` |
| Reviewed layout | `config/splat/` |
| Target-local symbols | `config/symbols/<target>.txt` |
| Reviewed Rizin annotations | `config/analysis/<target>/` |
| Authored lifts | `src/exe/`, `src/emi/` |

## Source and symbols

- Keep one `func_XXXXXXXX.c` per function and an adjacent `internal.h`.
- Use `func_80143B40` and `D_80143B40`; maps use sorted
  `name = 0xADDRESS;` entries with eight uppercase hex digits.
- Replace raw names only after review. Keep lifted filenames address-based.
- Edit `func_XXXXXXXX.c`, its target `internal.h`, the target-local map, and
  reviewed Splat boundaries as evidence improves. Keep declarations local
  unless a demonstrated cross-target contract requires sharing.
- Keep PsyQ external: use official declarations and target-local map evidence;
  never lift it or reuse an address across targets.
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
- After two members independently byte-match with the same C shape, move only
  the common body to `include/bof3/duplicates/<role>.inc`. Keep one
  address-based `func_XXXXXXXX.c` wrapper per member to provide its raw symbol
  and any compile-time parameters.
- Never reuse an extern address across targets. Each wrapper retains its
  target-local map, declaration, Splat boundary, and independent
  `asm-diff`/`byte-match` validation.

## Verification

- Use `bin/asm-diff` for instruction evidence and `bin/byte-match` for bytes.
- Run `bin/symbols check` after map edits; normalize only with
  `bin/symbols normalize [TARGET] --write`.
- Run `bin/decomp-status [TARGET...]` for the live lift audit.
- Run `just check` before handoff when practical; state skipped checks.
- Do not stage, commit, push, or mutate external systems without approval.

Use [matching](docs/matching.md), [Rizin evidence](docs/reverse-engineering.md),
and `$psx-rizin` for procedures. Store reviewed findings in `docs/specs/` and
reusable evidence-backed gotchas in `LESSONS.md`. Use the
[repository map](CONTEXT.md#repository-map) to locate tracked and ignored state.
