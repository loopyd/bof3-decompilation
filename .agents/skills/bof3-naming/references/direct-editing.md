# Direct naming edits

Use [editing and recovery](../../bof3-re/references/editing-recovery.md) for owned
PRE capture, adopted dirty work, native bounds, independent review and restoration.
First capture `bin/harness naming list TARGET --kind all --json` and live checks
for the selected identity and all affected callers. Record old/new spellings and
why the evidence supports the new name.

Keep address, storage extent/kind, width, signedness, volatility, layout, ABI,
compiler settings and function body unchanged for spelling-only work. Resolve all
uses in source, support bindings, headers, maps and reviewed analyzer annotations.
Update source filename, manifest claims, compiler flag keys and Splat source/name
metadata together when those paths or labels change. Preserve boundary addresses;
never use the filename to resolve identity. Remove the old spelling from the owned
surface without adding compatibility aliases.

Check composed maps, SDK ownership and deliberate typed aliases before changing a
binding. A same-address alias needs evidence; a map-normalization result proves no
semantic approval. Keep the naming baseline unchanged unless its maintenance is
explicitly authorized; never hide unresolved rows by accepting the current count.

After editing, run `bin/harness naming check TARGET [SELECTOR ...] --json`,
`bin/harness source symbols check TARGET`, `bin/harness source splat TARGET` and
relevant build/native checks. Naming checks discover direct identifier references.
For a header edit, resolve every transitive translation-unit consumer separately.
Check foreign consumers through their owning targets and retain the original
cross-target finding. Separate passing selected records from a failing global
report; neither a baseline update nor an unrelated passing check closes the gap.

Run the same native checks before and after each edit. Exact functions stay exact;
unselected partials preserve their complete measurements and compiled image.
Selected reconstruction work keeps its separately approved result constraints.

A retained partial spelling edit preserves its body, `@status partial`, `@match`
and `@residual` byte for byte, with unchanged live metrics and first mismatch.
Partial nonzero results are expected evidence, not passing exactness. Metadata
corrections follow the metadata owner after a successful live gate. On failure,
stop native jobs and restore only the owned PRE state; report unresolved recovery.
