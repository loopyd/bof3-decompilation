---
name: bof3-re
description: Execute BOF3 target-qualified function lifting, exact matching, duplicate-group normalization, and evidence-gated source promotion. Use when the user invokes `$bof3-re` to select or lift BOF3 functions, update reviewed target source, Splat layouts or maps, or promote proven cross-target duplicate bodies.
---

# BOF3 Reverse Engineering

Operate on one independently loaded BOF3 target and function at a time. Read
the repository `AGENTS.md`, then read `docs/usage.md`; read `docs/matching.md`
when editing or promoting C.

## Rules

- Qualify identity as `TARGET@0xADDRESS`; original bytes and PS-X headers win.
- Keep raw address-based filenames and target-local maps, declarations, and
  Splat ranges.
- Treat m2c, Rizin, signatures, rankings, and partial matches as hypotheses.
- Write readable C89; never use handwritten assembly to force a match.
- Add no tests for lifted game behavior. Add only the least tooling-contract
  test when tooling changes require one.
- Never commit unless the current user explicitly requests a commit.

## Choose scope

- Obey an explicit function or duplicate-group choice.
- When asked for guidance, show at most five `--detail minimal` candidates with
  effort, complexity, callers/callees, duplicate leverage, and confidence.
- When the user has not chosen, recommend but wait for their selection before
  editing. Do not silently choose a different scope.

## Lift

1. Validate the manifest, load address, reviewed boundary, and target map.
2. Run `bin/splat TARGET`, `bin/m2ctx TARGET@0xADDRESS`, and
   `bin/m2c TARGET@0xADDRESS -o out/candidate.c` as needed.
3. Edit the address-owned C file and only evidence-required local header, map,
   or Splat entries.
4. Iterate with `bin/asm-diff TARGET@0xADDRESS --detail normal`.
5. Accept only `bin/byte-match TARGET@0xADDRESS` equality.

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
contract to `include/bof3/core/`. Never cross-link EMIs or invent `src/engine/`
ownership.

## Verify and hand off

- Recheck every promoted member with `asm-diff` and `byte-match`.
- Run `bin/symbols check`, relevant Splat/build/status checks, and
  `git diff --check`; keep full evidence in `out/`.
- Report `Done`, `Evidence`, `Checks`, `Skipped`, and `Next` concisely.
- Suggest optional semantic or structural improvements for user choice.
