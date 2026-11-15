# Cleanup discovery

Inspect complete selected bodies and their producers/consumers before proposing
cleanup. Keep the combiner's grouping and native-preservation duties. Type,
qualifier and function-interface edits belong to `$bof3-types`; read its
[type refinement protocol](../../bof3-types/references/type-application.md).

For redundant `volatile`, follow the protocol's
[qualifier audit](../../bof3-types/references/type-application.md#volatile-qualifiers).
Separate pointer-cell and pointee qualifiers and identify hardware/asynchronous
writers before proposing removal. Register ordering alone is insufficient.

For a scratchpad pointer-table loop, follow the protocol's
[ABI investigation](../../bof3-types/references/type-application.md#scratchpad-tables-and-variadic-abi).
Trace the table's construction and all callers. Varying pointer shapes or element
counts alone do not prove variadic arguments. Keep unresolved interfaces unchanged.

Bind a bounded consumer group, preserve owned PRE and follow the types protocol's
checks and independent review. Report missing evidence and remaining work. A
discovery lead or rejected edit does not qualify a source refinement.
