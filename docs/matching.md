# Matching one function

Work on one `TARGET@0xADDRESS`; equal addresses in different targets are
unrelated until proven otherwise.

## Loop

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o candidate.c
# edit src/<target>/func_XXXXXXXX.c
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

1. Verify the target manifest, Splat boundary, and map.
2. Treat m2c output as a C seed, never layout evidence.
3. Recover control flow, signedness, access widths, calls, and delay slots in
   readable C89.
4. Update the function C, adjacent `internal.h`, target-local symbol map, or
   Splat boundary when the evidence requires it; rerun `bin/symbols check` and
   `bin/splat TARGET` after configuration changes.
5. Use `bin/asm-diff` for instructions and `bin/byte-match` for raw equality.
6. If semantics are credible but source shape differs, run one bounded
   `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` coordinator.

Permuter scores rank candidates; they do not accept a match. Do not run two
coordinators for one function.

## Reuse exact duplicate groups

Use `bin/rev-query duplicates TARGET@0xADDRESS --json` to inspect the complete
exact-byte candidate group. Match one deterministic representative, then
validate each reviewed member in its owning target.

Normalize names from evidence before sharing code:

- Use one semantic role for the group and the same names for equivalent
  parameters, locals, structs, and fields.
- Name unknown struct fields by offset (`unk_00`, `unk_04`) until behavior
  supports a semantic name.
- Keep addresses and raw function symbols target-local. Identical bytes do not
  make one module's extern address valid in another module.
- Keep constants as template parameters only when group members genuinely
  differ; exact members should normally use the same readable constants.

After two members independently match, the common body may live in
`include/bof3/duplicates/<role>.inc`. Each `func_XXXXXXXX.c` remains as a small
address-owned wrapper that defines the raw function macro and any explicit
parameters before including the template. Do not use a wrapper call or one
linked extern function: either can change instructions or cross independently
loaded binary ownership.

Every promoted member still requires its own source declaration, target map,
Splat `c` boundary, `bin/asm-diff`, and `bin/byte-match` result.

## Validate a candidate

```sh
bin/promote TARGET@0xADDRESS src/<target>/func_XXXXXXXX.c
```

After manually installing the candidate in its canonical source file,
`bin/promote` formats, compiles, links, compares, and prints required manual
edits. It never changes reviewed source, Splat, or maps.

## Audit lifts

```sh
bin/decomp-status [TARGET...]
bin/decomp-status exe/logo --json -o out/status.json
```

Results are `exact`, `partial`, or `invalid`. Valid partial lifts exit `0`;
invalid metadata, compilation, linking, or comparison exits `2`. Rizin-index
coverage is supplementary and may be unavailable without invalidating the live
lift audit.
