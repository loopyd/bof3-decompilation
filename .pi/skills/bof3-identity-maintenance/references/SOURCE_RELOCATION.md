# Source relocation

Relocation is an explicit `relocate-batch` transaction to `src/bof3/<class>/`; never relocate during matching or under another mode.

## Ownership and invariants

Every lift retains parsable function-level `@behavior` and address-authoritative `@source`; filenames never establish identity. Preserve `@behavior`, `@source`, `@kind`, evidence comments, boundaries, load addresses, SDK maps, and ABI. Use `/* @source 0x... @kind ... */`; `//` breaks gcc-2.6.3 objects.

Atomically update all affected source/support/header paths, manifest claims including `psyq_source`, Splat C-boundary `@source`, source-local include edges, and compiler flag keys. Do not move target configuration, alter `source_dir`, move public/shared headers or `src/shared/`, or edit `out/`, `build/`, or `toolchains/`. Other organization requires a plan and approval.

## Transaction

1. Refuse overlap with modified candidates unless the parent named those edits.
2. Consume the recursive inventory and manifest-less shared-config findings from [Naming audit v3](../../bof3-naming-evidence/references/NAMING_AUDIT_V3.md#recursive-inventory-and-audit-authority); do not redefine audit discovery while applying a move.
3. Validate metadata identity and destination class before moving anything. Never rename a Splat boundary address.
4. Apply the complete batch atomically. Any failed move, metadata update, regeneration, or validation reverts the whole batch; never fix forward.
5. Regenerate build metadata with `bin/build TARGET`, never by editing `build/`. Prove old paths absent from the graph and every current manifest source present.
6. Run `bin/symbols check TARGET`, `bin/splat TARGET`, fresh normal asm-diff and byte-match for every touched selector, then fresh `bof3-review`.
7. After authoritative map/Splat/reviewed/manifest changes pass, run both status commands. If either is stale, run one `bin/index --recover`, require both fresh, and never rebuild per file.
