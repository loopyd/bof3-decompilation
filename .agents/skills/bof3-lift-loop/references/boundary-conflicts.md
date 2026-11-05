# Boundary conflicts blocking the index

`analysis index` error
`conflicting function/global symbol: TARGET:NAME` means a reviewed Splat `asm`
subsegment claims a function where a header declares a global. Preserve error;
never weaken index validation, rename away conflict or edit generated output.

1. Run `bin/harness analysis boundary TARGET@0xADDRESS --probe-textbin -o out/<new>.json`.
   Reads manifests, headers, claims, Splat and original bytes directly; never opens
   or repairs index.
2. Classify complete original range: text, pointer/jump table, numeric table or
   code. Record decoding. Data in `.text` is not dialogue merely because unlifted.
   A boundary that never allocates its own frame, restores a frame it did not
   create, and starts with inherited saved registers is a switch arm, not a
   function: find the `jr`-free head (prologue plus jump table), and note where its
   segment boundary falls. A head inside a `bin` subsegment and a tail in the next
   segment means the function straddles a segment split, which is a parent-owned
   structural regroup - not a lane edit and not a boundary rename.
3. For finite unclaimed `asm` data, isolated before/after Splat splits must prove
   identical extraction, unchanged offsets and linker script unchanged except
   selected object path. `supported` probe is candidate evidence, not acceptance.
4. Apply only justified target-local `asm` -> `textbin` through `$bof3-re` ownership
   gates. Regenerate Splat, run `bin/harness analysis index` and `just index`.
5. Require both distinct review of classification/byte/layout preservation against
   original bytes and successful index commands. Neither substitutes for the other.
6. Retain original range hashes, probe workspace and resulting index rows. Splat
   leaves superseded output; delete only generated files absent from new linker script.

No unverifiable layout edits. Moving range to `rodata` shifts later symbols;
repository has no whole-image/runtime oracle for that change.
