# Matching one function

Compile and compare one target-qualified PSX/MIPS function. Exact instruction
and raw-byte equality are separate facts.

## Quick path

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS > candidate.c
# edit src/<target>/func_address.c
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

`TARGET@0xADDRESS` prevents same-address functions in independent overlays
from being confused. The owned source path is always
`src/<target>/func_XXXXXXXX.c`.

## Loop

1. Confirm the manifest image, load address, Splat split, and map with
   `bin/symbols check` before changing C.
2. Use `bin/m2ctx` and `bin/m2c` for a target-local C seed. m2c output is a
   starting point, not a source of layout facts.
3. Fix boundary, signedness, access width, calls, and delay-slot behavior in
   readable C89.
4. Run `bin/asm-diff TARGET@0xADDRESS` for the vendored asm-differ result and
   `bin/byte-match TARGET@0xADDRESS` for independent raw equality.
5. If control flow is credible but source shape remains wrong, use one bounded
   coordinator: `bin/permute TARGET@0xADDRESS --time-limit 300`.

`bin/permute` owns a deterministic workspace below `out/permuter/`; do not run
two coordinators for the same function. Its score ranks candidates but never
accepts a match.

## Candidate review

```sh
bin/promote TARGET@0xADDRESS candidate.c
```

Promotion is validate-only. It formats, compiles, links at the original
address, runs both comparison tools, and prints any required manual source,
Splat, or map edits. It never copies a candidate into `src/` or mutates tracked
configuration.

Use `--json` for structured output and `--example` for an exact invocation.
Exit 0 means success/match, 1 is a valid nonmatch or pending write, and 2 is a
usage, configuration, or tool failure.
