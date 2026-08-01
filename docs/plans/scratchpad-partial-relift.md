# Scratchpad fallback and partial re-lift handoff

**Status:** active

## Goal

Make `bin/scratchpad` publish a useful, public decomp.me payload for an existing
partial lift even when its reviewed Splat entry emits no `asm/func_*.s`, and
make its C context include the public declarations needed by source includes
outside `internal.h`. Add the partial-lift stop-and-share handoff to the lift
loop.

## Evidence baseline

- `emi/battle/battle/03@0x801D0C00` is a partial map entry whose range begins
  with `0x410` bytes of data; it has no Splat `func_801D0C00.s`, so current
  `bin/scratchpad preview` fails before it can publish the mandated escalation.
- `DecompMeScratchpadToolchain._minimal_context` only retains directly named
  structs and `extern` declarations from preprocessor output. It does not close
  type dependencies or explicitly expose a reusable declaration-lookup seam.
- `canonical.load_target_symbols` is the single authority for composed map
  ownership, but maps cannot supply C prototypes/types.

## Phase 1 — public declaration context

1. Add one small parser/resolver module for public preprocessed declarations.
   It must select source-referenced declarations from every project header,
   recursively include referenced typedef dependencies, preserve stable source
   order, and reject unresolved references to ignored PsyQ/toolchain headers.
2. Replace scratchpad-local declaration regex selection with that resolver.
   Keep primitive aliases and do not export paths, user inputs, toolchain text,
   or unused declarations.
3. Add focused unit tests for direct and transitive header declarations and
   ignored-header rejection.

## Phase 2 — non-mutating original-byte fallback

1. Keep the reviewed Splat assembly path as the preferred scratchpad target.
2. If it is absent, infer the established function/range size using the existing
   matching resolver, extract original bytes from the target binary, and render
   a valid `.text`/`glabel` assembly payload as endian-correct `.word` rows.
   Do not generate Splat, alter map/layout/source, or pretend data is C code.
3. Add a focused partial-lift payload test using `battle/03@801D0C00` and retain
   the existing Splat-backed payload test.

## Phase 3 — partial-loop stop handoff

1. Extend `.pi/skills/bof3-lift-loop/SKILL.md`: after normal bounded candidates,
   process a fresh partial catalog serially; exact results retain through the
   normal review gate.
2. On the first non-exact partial result, restore the prior partial state,
   invoke `bin/scratchpad share TARGET@0xADDRESS`, record the URL/result in the
   loop journal, and stop. A publish failure is also a stop/report condition.

## Validation

```sh
python -m pytest -q tools/python/tests/test_scratchpad.py
bin/scratchpad preview emi/battle/battle/03@0x801D0C00
bin/scratchpad share emi/battle/battle/03@0x801D0C00
just check
```

## Boundaries

- `out/` remains disposable; never commit generated assembly or scratch URLs as
  reviewed reverse-engineering facts.
- No new dependency, map/Splat mutation, header invention, toolchain content,
  or private PsyQ declaration may enter a public payload.
- The fallback represents canonical bytes as data words when no reviewed asm
  exists; a follow-up boundary correction needs independent Splat review.
