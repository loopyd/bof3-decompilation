# Type refinement

## Inspect and prove

Use `bin/harness types list TARGET --kind all --json` for declarations, fields,
constraints, conflicts, lexical uses and original-access storage leads. Listings
require a fresh existing index and never refresh it. A display limit does not
reduce check coverage. Refresh disposable evidence explicitly only at an authorized
checkpoint after authoritative edits pass.

Inspect target/shared headers and PsyQ definitions before adding aliases,
aggregates, fields or prototypes. Keep same-spelling target-private types distinct.
Resolve tags, typedef aliases, pointer/array aliases and forward declarations by
identity; a pointer alias does not inherit pointee layout. Unknown layout remains
unknown until original accesses and independent caller/consumer evidence settle it.

Prove width, signedness, pointer depth, alignment, offsets, qualifiers and ABI.
Distinguish storage section from representation and semantic role. Unaligned words
may need `u8[N]`; calculate padding from the next free byte and verify actual layout.
Reuse SDK `MATRIX` when evidence fits. Adjacent stack blobs may be separate locals.
Pointer-cell volatility differs from pointee volatility; each needs asynchronous
or hardware evidence. Use hex for addresses/masks/encoded values and decimal for
human quantities.

## Edit and check

Follow [editing and recovery](../../bof3-re/references/editing-recovery.md) before
writes. Record proposed representation, supporting observations, owned files and
all consumers, including support sources, shared headers and every grouped member.
Follow each source's effective compiler, flags, include search and forced includes;
lexical absence alone does not exclude a preprocessor consumer.

Apply the authorized representation directly in its owning header/source. Keep
storage extent, initialization, address ownership and unrelated declarations fixed.
Update every affected field access and prototype consistently. Check
`bin/harness types check TARGET [SELECTOR ...] --json`, source/map/layout consistency
and the existing build/native gates. Measurable layout conflicts fail; unsupported
or missing evidence remains unavailable. Native exactness proves bytes over the
complete reviewed original range, not the semantic correctness of a type.

For shared promotion, first establish independently reviewed private evidence in
at least two distinct targets with the same address-free representation and
semantic contract. Prove shared ownership and include dependency coverage, then
check every affected target explicitly. A single compile or identical spelling
cannot establish shared suitability. Preserve existing frozen campaign evidence
and its unfinished private/shared review duties.

## Mixed reconstruction and partials

A type change may include C reconstruction only when that work is authorized and
its expected POST result is independently reviewed before edits. Keep existing
exact consumers exact. Preserve every unselected partial's compiled image,
measurements, first mismatch and progress metadata. Selected partials must meet the
reviewed expected result, never decrease matching instructions or increase absolute
size difference, and receive semantic review of every changed instruction. A higher
percentage alone cannot qualify an edit. Record honest partial residuals.

Require independent final review of representation, ABI and all affected consumers.
On native or review failure, stop owned writers before restoring only owned PRE.
Uncertain cleanup/restoration remains a blocker; preserve evidence and report it.
