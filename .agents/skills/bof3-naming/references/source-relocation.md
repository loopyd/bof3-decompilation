# Source relocation

Use an explicitly selected relocation batch. Put game sources in the existing
`src/bof3/<subsystem>/` folder that matches their reviewed role. Keep target identity
in metadata and claims rather than adding target directory levels. Include changes
to shared code or headers only when the selected plan covers their full consumers.

## Ownership and invariants

Every lift retains parsable function-level `@behavior` and address-authoritative
`@source`; filenames never establish identity. Preserve `@behavior`, `@source`,
`@kind`, evidence comments, boundaries, load addresses, SDK maps and ABI. Use block
comments for metadata; `//` breaks gcc-2.6.3 objects.

Migrate affected source/support/header claims, Splat C-boundary `@source`, local
include edges and compiler flag keys together. Preserve each effective compiler
and ordered flags. Keep private headers under `include/bof3/<subsystem>/`, public
headers under `include/`, and shared implementations under `src/shared/<subsystem>/`.
Preserve justified root headers and generated `psyq_source` ownership. Keep target
configuration and `source_dir` in place. Regenerate build outputs through their
owners; never hand-edit `build/` or toolchains to repair a move.

## Direct batch

1. Stop on modified candidate overlap unless the parent adopted those edits.
2. Inspect `bin/harness naming list TARGET --kind files --json`, manifest claims,
   include edges and compiler flag keys. Follow [direct editing](direct-editing.md)
   for owned PRE and recovery.
3. Validate metadata identity and the destination subsystem before moving files.
   Preserve every Splat boundary address.
4. Apply the complete batch. On failure, keep it applied, preserve evidence and
   follow [editing and recovery](../../bof3-re/references/editing-recovery.md#failure-and-continuation).
   Fix the cause and recheck affected gates before starting another candidate.
5. Run `bin/harness build TARGET` to regenerate build metadata. Verify that the
   graph omits old paths and includes every current manifest source.
6. Run `bin/harness source symbols check TARGET`, `bin/harness source splat TARGET`
   and fresh asm-diff/byte-match for every touched selector. Obtain independent
   review of the complete batch.
7. After those gates pass, run `bin/harness analysis readiness TARGET`. If stale,
   run one `bin/harness analysis index` and require fresh readiness.
   Refresh once per batch rather than once per file.
8. Remove obsolete paths after all consumers migrate. Prune only confirmed empty,
   unclaimed directories. Keep any directory that still owns unrelated files.
