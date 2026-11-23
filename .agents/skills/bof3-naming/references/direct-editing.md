# Direct naming edits

Use [editing and recovery](../../bof3-re/references/editing-recovery.md) for owned
PRE capture, adopted dirty work, native bounds, independent review and recovery.
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
follow [failure and continuation](../../bof3-re/references/editing-recovery.md#failure-and-continuation).
Confirm writer termination, keep the batch applied and preserve evidence while
fixing the cause. Recheck affected gates and report unresolved recovery.

## Rename transaction

A spelling-only rename of an already-authored row uses the harness transaction
instead of hand edits: `bin/harness naming rename TARGET OLD NEW` plans, and
`--apply` writes. The skill still supplies both spellings and their evidence; the
transaction applies no semantic choice.

The dry run is mandatory input: it lists the filename, identifier, target-owned
callers and declarations, Splat label and `@source`, symbol row, target claim and
the per-object compiler key in `config/compiler/object-flags.cmake` it would
rewrite. A unit with its own `BOF3_OBJFLAGS_`/`BOF3_OBJCOMPILER_` key rebuilds
shorter without it, so the key follows the file. A refusal names its cause and writes nothing: a new spelling
that breaks the [name schema](naming-style.md), a composed-map collision, a
`config/targets/shared/symbols.txt` claim, or a consumer outside the owning
target. Fan-out is a parent decision, not a rename.

`--apply` holds the writer lease and proves the rename by measuring the source
before and after: an exact body stays exact, a partial keeps its own score. A
refused proof restores the previous spelling and names the selector. A rejected
rename is undone by running the same transaction with the two spellings swapped.

A rename wave changes a target's config, so its Rizin snapshot goes stale.
Refresh the index (`bin/harness analysis index`) before another readiness or
analysis query reads that target; the stale recipe is reported, never refreshed
silently.
