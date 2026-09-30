# Boundary conflicts blocking the index

`analysis index` or `analysis index --recover` error
`conflicting function/global symbol: TARGET:NAME` means a reviewed Splat `asm`
subsegment claims a function where a header declares a global. Preserve error;
never weaken index validation, rename away conflict or edit generated output.

1. Run `bin/harness analysis boundary TARGET@0xADDRESS --probe-textbin -o out/<new>.json`.
   Reads manifests, headers, claims, Splat and original bytes directly; never opens
   or repairs index.
2. Classify complete original range: text, pointer/jump table, numeric table or
   code. Record decoding. Data in `.text` is not dialogue merely because unlifted.
3. For finite unclaimed `asm` data, isolated before/after Splat splits must prove
   identical extraction, unchanged offsets and linker script unchanged except
   selected object path. `supported` probe is candidate evidence, not acceptance.
4. Apply only justified target-local `asm` -> `textbin` through `$bof3-re` ownership
   gates. Regenerate Splat, run `bin/harness analysis index --recover` and `just index`.
5. Require both distinct review of classification/byte/layout preservation against
   original bytes and successful index commands. Neither substitutes for the other.
6. Retain original range hashes, probe workspace and resulting index rows. Splat
   leaves superseded output; delete only generated files absent from new linker script.

No unverifiable layout edits. Moving range to `rodata` shifts later symbols;
repository has no whole-image/runtime oracle for that change.
