# Compiler variant tooling

Read [the agent index](INDEX.md) for workflow routing. Compiler provenance and
target-specific results remain in [game research](../specs/runtime/compiler-variants.md).

## Ownership

- `config/compiler/variants.json` — schema: `harness.compiler-variants/v1`
- `bin/compiler-variants` — CLI list/install/verify/path
- `tools/python/harness/toolchain/gcc_variants.py` — `CompilerVariant` / `EmptyCatalog`
- `tools/python/harness/build/compiler.py` — per-object flag/compiler parsing for CMake parity
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
<id>` and `compile_commands.json` resolve a selected compiler through the same
ensure-installed operation; a missing install self-heals from the verified
cache, while an unsupported host, unknown ID, corrupt install, or failed
install fails closed (never canonical/host GCC). `just setup` primes the
canonical compiler plus every host-compatible entry in the
`config/compiler/variants.json` catalog; host-incompatible candidates are
skipped with their ID and host reported, an invalid catalog fails setup
closed, and a catalog with no candidates installs nothing. Setup never sets
`PSX_GCC`, adds an object override, or changes the default compilation
selection. Doctor remains verification-only. No
compiler ID is accepted by the build unless it appears in the catalog.

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
