# Source relocation

Explicit `relocate-batch` only, destination `src/bof3/<class>/`. No relocation
during matching or another mode.

## Ownership and invariants

Every lift retains parsable function-level `@behavior` and address-authoritative `@source`; filenames never establish identity. Preserve `@behavior`, `@source`, `@kind`, evidence comments, boundaries, load addresses, SDK maps, and ABI. Use `/* @source 0x... @kind ... */`; `//` breaks gcc-2.6.3 objects.

Atomically update all affected source/support/header paths, manifest claims including `psyq_source`, Splat C-boundary `@source`, source-local include edges, and compiler flag keys. Do not move target configuration, alter `source_dir`, move public/shared headers or `src/shared/`, or edit `out/`, `build/`, or `toolchains/`. Other organization requires a plan and approval.

## Transaction

1. Modified candidate overlap blocks unless parent explicitly adopted edits.
2. Consume the recursive inventory and manifest-less shared-config findings from [Naming audit v3](naming-audit-v3.md#recursive-inventory-and-audit-authority); do not redefine audit discovery while applying a move.
3. Validate metadata identity and destination class before moving anything. Never rename a Splat boundary address.
4. Apply complete batch atomically. Move/metadata/regeneration/validation failure
   reverts whole batch; never fix forward.
5. Regenerate build metadata with `bin/harness build TARGET`, never by editing `build/`. Prove old paths absent from the graph and every current manifest source present.
6. Run `bin/harness source symbols check TARGET`, `bin/harness source splat TARGET`, fresh normal asm-diff and byte-match for every touched selector, then fresh `bof3-reviewer`.
7. After authoritative map/Splat/reviewed/manifest changes pass, run both status commands. If either is stale, run one `bin/harness analysis index --recover`, require both fresh, and never rebuild per file.
