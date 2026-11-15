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

## Declarations and casts

Let local, global and field declarations carry proven types instead of repeated
cast chains. Keep explicit C89 types. Implicit-int declarations and undeclared
function calls are not type inference. Before removing a cast, establish the
operand and result types, promotions, arithmetic width, signedness, pointer depth,
qualifiers, aliasing and ABI. Retain required conversions, including narrowing and
integer-address-to-pointer conversions. A shorter expression does not prove a cast
redundant; unknown types remain evidence gaps.

Collect local declarations at function start where scope, shadowing, lifetime and
native preservation permit. Keep storage duration and initialization at their
original execution points. Separate an initializer from its declaration only when
an assignment is legal and equivalent in C89. Preserve nested declarations when
hoisting would change binding, object lifetime, initialization, a retained partial
body or the complete native result. Check every read still follows initialization.

## Aggregate identity and variants

Matching primitive fields are a discovery lead. Reuse one struct definition only
when original accesses and producers/consumers prove the same layout and role.
Compare extent, alignment, offsets, padding, signedness, pointer depth, qualifiers
and ownership. Keep independent objects and distinct semantic types separate.
Use typedef aliases for proven duplicate definitions; a union does not deduplicate
independent storage.

Use a union only for proven alternative views of the same storage. Give variants
their own typedefs and use named union members for C89. Establish how each variant
becomes active and which accesses are valid during its lifetime. Preserve
size, alignment, field offsets, initialization, aliasing and argument/return ABI.
Do not infer safe cross-member reads or overlay ownership from equal footprints.
Unproven variants remain unresolved. Shared promotion still needs the independent
private-target proofs below.

## Volatile qualifiers

Audit each storage object before removing `volatile`. Identify every writer and
reader, including interrupt callbacks, hardware registers and DMA/device activity.
CD-ROM callbacks are one asynchronous case, not the only one. Separate volatility
of a pointer cell from volatility of its pointee; prove each qualifier independently.
Ordinary RAM, scratchpad placement or synchronous-looking callers alone do not
settle whether hardware or another execution context can change the value.

Remove a qualifier only when the complete access and mutation evidence proves it
unnecessary. Preserve observable access width, count and ordering, plus required
reloads and storage ownership. Correct register scheduling and identical current
bytes do not establish the storage contract. Keep required hardware/asynchronous
qualifiers. Do not add casts, barriers or register controls to recover a match.
Missing writer or timing evidence leaves the removal open.

## Scratchpad tables and variadic ABI

A function that loops over differently shaped pointers through its sole argument
may consume a fixed table. Scratchpad access is an investigation lead, not proof
of variable function arguments. Trace the table's producers and every direct or
indirect caller alongside the callee's original accesses and declarations.

Establish argument counts, register/stack locations, promoted types, traversal
order and the count or sentinel that bounds consumption. Distinguish an explicit
table pointer from arguments saved or copied into scratchpad. Adopt `<stdarg.h>`,
`va_start`, `va_arg` and `va_end` only when the effective compiler's variadic ABI
fits all affected callers and the complete callee. C89 needs a valid last named
parameter; do not invent one or read an argument with the wrong promoted type.
Keep the evidenced fixed-argument interface when that proof is absent. Reopen a
rejected ABI experiment only when new evidence changes its failed premise.

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
