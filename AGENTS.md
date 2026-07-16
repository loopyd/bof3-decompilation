# AGENTS.md — BOF3

BOF3 is a set of independently loaded binaries, not one link target. Work on
one exact target and function at a time.

## Ownership

- Work only in this repository. `inputs/` is ignored user media and is never
  committed.
- `out/`, `build/`, and `toolchains/` are generated. Do not hand-edit them or
  add another generated-artifact root.
- `config/targets/<target>.toml` owns binary identity, image, and load address.
  `config/splat/` owns reviewed segment layouts. `config/symbols/<target>.txt`
  owns target-local symbols. `config/analysis/` owns reviewed Rizin replay.
- Analyze the executable image or extracted EMI entry, never an EMI archive.
  Identical addresses or bytes in different targets remain separate facts.
- Original bytes and PS-X headers outrank analyzer output. Verify PS-X `t_addr`;
  do not assume `0x80010000`.

## Source and symbols

- Lifted source is one `func_XXXXXXXX.c` per function with an adjacent
  `internal.h`. `src/exe/` owns executables; `src/emi/` owns EMI entries.
- Raw functions are `func_80143B40`; raw data is `D_80143B40`. Hex is eight
  uppercase digits in maps and documentation. Semantic/PsyQ names replace raw
  map names after review; lifted filenames stay address-based.
- Maps use `name = 0xADDRESS;`, sorted by address. Run `bin/symbols check`;
  normalize only with `bin/symbols normalize [TARGET] --write`.
- Generated weak bindings are `out/bindings/<target>/symbols.c`. Never edit or
  track them. PsyQ is external code: use official declarations and target-local
  map entries; do not lift it or reuse its address across binaries.
- Write readable C89. Do not use handwritten assembly to force a match.

## Daily loop

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS > candidate.c
# edit src/<target>/func_address.c
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
bin/permute TARGET@0xADDRESS --time-limit 300
bin/promote TARGET@0xADDRESS candidate.c
```

`bin/promote` validates a candidate only: it formats, compiles, links, diffs,
and byte-checks, then prints the manual edits required. It does not modify
reviewed source, layouts, or maps. Run one permuter coordinator per function.

## Rizin and evidence

- Use `bin/rz-project open|status|rebuild|export|analyze TARGET`. Each target
  gets an isolated generated project under `out/`; never combine overlapping
  binary mappings.
- `export` prints a deterministic patch. `--write` is the explicit mutation
  path after validation. Deep analysis makes candidates, not reviewed facts.
- Build the cross-target cache only with `just index`; query it with
  `bin/rev-query`. A stale or incomplete Rizin export must fail indexing.
- Use the retained `$psx-rizin` skill for target-qualified analyzer procedure.
  Put stable findings in `docs/specs/` and reusable evidence-backed gotchas in
  `LESSONS.md`.

## Verification

- Use the narrowest check while iterating: `bin/asm-diff` for instruction
  evidence and `bin/byte-match` for raw equality. Nonmatches are normal and
  exit 1; usage/config failures exit 2.
- Before handoff, run `just check` when practical and state skipped checks.
- Do not stage, commit, push, or mutate external systems without approval.
