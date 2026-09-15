# Compiler variant tooling

Read [the agent index](INDEX.md) for workflow routing. Compiler provenance and
target-specific results remain in [game research](../specs/runtime/compiler-variants.md).

## Ownership

- `config/compiler/variants.json` — schema: `harness.compiler-variants/v1`
- `bin/compiler-variants` — CLI list/install/verify/path
- `tools/python/harness/toolchain/gcc_variants.py` — `CompilerVariant` / `EmptyCatalog`
- `tools/python/harness/build/compiler.py` — attached metadata, path defaults and ordered compiler arguments
- `tools/python/harness/commands/compiler_variants.py` — CLI commands

The `variants.json` catalog is reviewed, tracked metadata — the single source
of truth for compiler IDs, archive digests, and executable paths. GCC archives
are cached, SHA-256-verified, under
`inputs/external/private-assets/toolchains/gcc/`; installed variants live in
ignored local state under `toolchains/gcc-variants/` and the canonical
compiler under `toolchains/gcc-2.7.2-psx/` (unrelated PSn00b/Rizin downloads
stay in `toolchains/downloads/`). Canonical GCC and every catalog variant share
one archive lifecycle: cache-symlink/non-regular rejection, cache-local
temporary download, digest validation before atomic cache publication, fresh
sibling staging extraction, staged `gcc --version` identity verification, and
an atomic install swap that preserves a prior verified install on any failed
network, digest, extraction, or identity check. `bin/compiler-variants path
<id>` and compilation database generation share installed-only resolution:
they verify the selected compiler but never download, install or repair it,
even when a cached archive exists. Missing installations report the explicit
`bin/compiler-variants install <id>` remedy; run it only with installation
authorization. Unsupported hosts, unknown IDs and corrupt installations reject,
never falling back to canonical/host GCC. `just setup` primes the
canonical compiler plus every host-compatible entry in the
`config/compiler/variants.json` catalog; host-incompatible candidates are
skipped with their ID and host reported, an invalid catalog fails setup
closed, and a catalog with no candidates installs nothing. Setup never sets
`PSX_GCC`, adds an object override, or changes the default compilation
selection. Doctor remains verification-only. No
nondefault compiler ID is accepted by the build unless it appears in the catalog.

## Function compiler settings

Attached function metadata may select `@compiler <catalog-id>` or `@gcc <version>`
and `@flags <ordered-flags>` or `@cflags <ordered-flags>`. Put each setting on its
own line in the same leading comment as `@source` and `@behavior`, immediately
above the implementation. Compiler tags require attached records even in a
single-function file; legacy metadata before includes remains readable only
without these settings. Duplicate settings, orphan tags and ambiguous records
reject. Strings containing tag-like text are not metadata.

`@gcc 2.6.3` selects `gcc-2.6.3-psx` from the catalog; `@gcc 2.7.2` or
`@compiler gcc-2.7.2-psx` explicitly selects the canonical compiler. Metadata
overrides the legacy path-keyed setting, which otherwise overrides project
defaults. Flags replace the canonical optimization level, retain the other base
flags and preserve their supplied order before the standard assembler options.
Use nonempty, whitespace-separated literal flag tokens; shell quoting,
substitution and CMake separators are unsupported. Effective profiles reject
operation/output-control options such as `-E`, `-S` and
`-o`, including inherited path overrides. The shared strict configuration reader
also rejects bracket comments, multiline or quoted assignments, trailing comments
and duplicate keys rather than interpreting them differently from CMake. Every
member in one translation unit must resolve the same compiler and ordered override, including
members that inherit defaults. This is not permission to change a reviewed
function's effective profile without new byte evidence.

CMake inventory v4, compilation database generation and preservation profiles
share this resolver. The inventory records effective settings, so annotation
changes invalidate configuration even when source grouping is unchanged.
Direct annotated `bin/cc` calls select the metadata compiler when `PSX_GCC` is
absent and require the complete configured `-c SOURCE -o OBJECT` argument vector;
conflicting environment selections, flags, custom drivers and other modes reject
before launch rather than silently ignoring metadata. The compiler environment
uses that same selected executable. Installed-only CLI resolution verifies identity;
dispatch itself does not perform another version check.

`bin/flag-search` explicitly marks its existing scratch directory through
`BOF3_COMPILER_TRIAL`. Only one ungrouped scratch object may use trial overrides;
the marker is bound to the dispatch and cannot relax grouped preservation checks.
Trial results do not update authored metadata or establish configured production
acceptance. Preservation v4 retains PRE source text authenticated by its original
hash, so removed or overwritten paths cannot erase the original settings.
Captured records must fit the existing 4 MiB document bound, including their PRE
text; oversized records reject before publication. Profile v2 optionally embeds a
hash-pinned destination draft, resolves its attached metadata without writing it,
and requires identical compiler/ordered arguments and exact selected membership.
Capture and verification bind POST bytes to that draft; this does not migrate
legacy configuration, prove C semantics or authorize consolidation.

## Verification

```sh
# Check catalog state
bin/compiler-variants list
bin/compiler-variants verify <id>

# Verify baseline build unchanged
just check
bin/symbols check
```

`list` reports catalog membership; `verify <id>` validates an ignored local
installation. Unoverridden objects keep the canonical compiler. Reviewed object
selections and dated negative matrices stay in [game research](../specs/runtime/compiler-variants.md),
not a second selection inventory here. Historical checks below are not live proof.

## Historical checks (2026-07-30)

| Test file | Tests | Status |
|-----------|-------|--------|
| `test_bin_cc_pipeline.py` | 3 hermetic stub tests for GCC→maspsx→assembler arg flow | PASS |
| `test_asm_link.py` | 4 fixture-local tests for relocation-aware linking + byte extraction | PASS |
