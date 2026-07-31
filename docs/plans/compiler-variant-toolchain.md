# Managed historical GCC variants

**Status:** Phase 0–2 complete (empty-catalog framework)
**Scope:** add an opt-in, provenance-pinned way to compare one lifted function
against managed historical PS1 GCC variants. The canonical compiler remains
`gcc-2.7.2-psx -> maspsx -> ASPSX 2.56 emulation`.

## Goal and evidence baseline

`emi/battle/battle/15@0x800AF66C` exhausted its readable-C, supported-flag,
permuter, and bounded local-pin ladders. Its strongest clean-C candidate is
same-size but has an entry-register allocation mismatch. The original overlay
contains no debug/compiler provenance strings, so its compiler version and
options are unproven.

The current project chain is intentionally canonical, but
[`docs/specs/runtime/compiler-provenance.md`](../specs/runtime/compiler-provenance.md)
already proves that several old GCC variants do **not** solve the closed
`exe/slus_004_22@0x80162B08` residual. This effort must not reopen that
function or silently change normal builds.

Existing ownership seams:

- `tools/python/harness/toolchain/gcc.py` installs the canonical GCC.
- `bin/cc` already accepts an explicit `PSX_GCC` executable while continuing to
  own GCC environment setup, maspsx, and assembler dispatch.
- `bin/flag-search` uses `compile_commands.json` and relocation-aware function
  byte comparison in a temporary workspace.
- `config/compiler/object-flags.cmake`, `CMakeLists.txt`, and
  `compile_commands.py` already synchronize verified per-object flags.
- `toolchains/` and downloads are ignored. Only metadata, code, tests, and docs
  are reviewed/tracked.

## Non-goals and invariants

- Do not vendor compiler archives/binaries, source user media, or proprietary
  PsyQ compiler files.
- Do not change `bin/cc`, `just setup`, the default toolchain, or normal build
  commands when no object selects a variant.
- Do not add a generic compiler wrapper, target-wide compiler setting, automatic
  downloads, a batch sweep, or a matching macro.
- Do not retain a compiler profile from score, assembly resemblance, or raw
  object similarity: the specified `TARGET@0xADDRESS` must pass fresh
  `bin/byte-match` with clean C.
- A catalog record requires an authoritative identity/source, license status,
  immutable artifact or reproducible build recipe, SHA-256, expected executable
  identity, supported host constraints, and current maspsx/ASPSX compatibility.
  Never guess URLs, checksums, or redistribution rights.

## Phase 0 — research and freeze the canonical control

**Affected files:** no production changes. Later record durable findings in
`docs/specs/runtime/compiler-variants.md` only when evidence is verified.

1. Identify authoritative, redistributable or reproducibly-buildable variants
   nearest to the canonical chain. The initial research matrix is:

   | Priority | Provisional ID | Purpose | Restriction |
   | --- | --- | --- | --- |
   | control | canonical `gcc-2.7.2-psx` | reproduce the existing result | existing default only |
   | 1 | `gcc-2.7.2-psx-r1` through `-r3` | nearest scheduler/patch deltas | research all provenance first |
   | 2 | `gcc-2.7.1-psx`, `gcc-2.7.0-psx` | adjacent release comparison | test only after priority 1 |
   | 3 | `gcc-2.6.3-psx`, `gcc-2.6.0-psx` | older bounded controls | stop if nearer variants are non-exact |

   GCC 2.5.7, GCC 2.8+, stock PsyQ CC1PSX, and the closed SLUS residual are
   explicitly out of scope.

2. For each usable candidate, collect the source/release identifier, exact
   acquisition URL or build recipe, SHA-256, license/notice, host requirements,
   executable-relative path, `--version` expectation, and evidence it works with
   current `-gcoff`, MIPS flags, maspsx, and ASPSX 2.56 emulation.
3. Capture a disposable normal-build control in `out/`: canonical compiler
   identity, `just build` result, and a sorted SHA-256 manifest of built objects.
   This is comparison evidence only and remains untracked.
4. Select one *live*, ladder-qualified pilot. `0x800AF66C` is blocked until a
   maintained source candidate exists again; do not use an absent source or a
   cached permuter artifact as a pilot.

**Acceptance:** all proposed candidate facts are independently sourced; the
canonical control validates; no candidate lacking provenance advances.

**Stop:** if no safe, compatible, verifiable candidate exists, publish the
negative evidence and stop without code changes.

**Decision (2026-07-23):** No safe, compatible, verifiable historical GCC
candidate was found during research. Negative evidence is published in
`docs/specs/runtime/compiler-variants.md`. This plan authorizes building the
empty-catalog framework (Phase 1+) so that when a candidate later appears,
the infrastructure exists to validate it safely — without altering canonical
compiler behavior or inventing compiler provenance.

## Phase 1 — catalog and managed variant lifecycle

**Affected files:**

- add `config/compiler/variants.json`;
- update `.gitignore` and `toolchains/README.md`;
- update `tools/python/harness/io.py`;
- add candidate-only lifecycle code under `tools/python/harness/toolchain/`;
- add focused tests under `tools/python/tests/`.

1. Add a strict, versioned catalog for non-default compilers. Each safe unique
   ID maps to its Phase-0 provenance, digest, archive/source recipe,
   executable-relative path, expected identity, host constraint, license notice,
   and assembler assumptions.
2. Give `RepoLayout` one ignored install root:
   `toolchains/gcc-variants/<id>/`; reuse ignored `toolchains/downloads/` for
   archives. Preserve the canonical `gcc-2.7.2-psx` root and its installer.
3. Implement candidate resolution, download/build, digest verification,
   path-safe extraction, executable/identity verification, and safe-path checks
   in the Python managed-toolchain layer. Fail closed before execution on an
   unknown ID, traversal path, bad digest, wrong identity, or unsupported host.
4. Add one thin `bin/compiler-variants` dispatcher with only:
   `list`, `install <id>`, `verify <id>`, and `path <id>`. Installation remains
   explicit; candidates must not join `managed_toolchains()`, `just setup`, or
   baseline doctor setup.

**Validation:**

```sh
python -m pytest -q tools/python/tests/test_gcc_variants.py
bin/compiler-variants list
bin/compiler-variants install <researched-id>
bin/compiler-variants verify <researched-id>
bin/compiler-variants path <researched-id>
just doctor
just check
```

**Acceptance:** invalid catalog/install inputs fail safely; an empty catalog is
valid; plain `bin/cc`, `just setup`, and `just doctor` retain canonical behavior.

**Rollback:** delete ignored candidate archive/install state only. Do not alter
canonical tooling.

## Phase 2 — explicit variant comparison in `flag-search`

**Affected files:**

- `tools/python/harness/commands/flag_search.py`;
- `tools/python/harness/match/flag_search.py`;
- focused `tools/python/tests/test_flag_search.py` coverage;
- `docs/matching-playbook.md` and `docs/specs/runtime/compiler-quirks.md`.

1. Add repeatable `--compiler <catalog-id>` to `bin/flag-search`. With no such
   option, preserve the current arguments, JSON schema, canonical compiler, and
   results exactly.
2. Resolve and verify every explicitly requested variant before compiling. For
   each `(variant, flag profile)` pair, invoke the existing compile command via
   `bin/cc` with `PSX_GCC` set only in that subprocess environment; preserve the
   existing temporary output and relocation-aware comparison path.
3. Emit candidate ID, verified identity, digest, flags, byte-match status, and
   instruction percentage. The command must not download implicitly, write
   `object-flags.cmake`, change CMake configuration, or mutate `build/`.
4. Reject an unavailable selected compiler clearly rather than falling back to
   host or canonical GCC.

**Validation:**

```sh
just build
bin/flag-search TARGET@0xADDRESS
bin/flag-search TARGET@0xADDRESS --compiler <researched-id>
python -m pytest -q tools/python/tests/test_flag_search.py
```

**Acceptance:** tests show subprocess-only `PSX_GCC` selection and no-option
behavior is unchanged. Search output alone is never promotion evidence.

**Stop:** abandon a candidate immediately for compile/link failure, size/CFG
divergence that contradicts the diagnosed residual, or a non-exact result. Do
not expand the matrix merely because a score improves.

## Phase 3 — opt-in, object-local retention after an exact result

**Affected files:**

- `CMakeLists.txt`;
- `config/compiler/object-flags.cmake`;
- `tools/python/harness/commands/compile_commands.py`;
- focused CMake/build/compile-command/doctor tests;
- matching and toolchain documentation.

1. Extend the existing per-object configuration with optional
   `BOF3_OBJCOMPILER_<sanitized-source-key> <catalog-id>` entries. It is valid
   only alongside evidence comments naming the exact target/function, candidate
   identity/digest, flags, and live verification.
2. CMake resolves only allowlisted catalog IDs. For one selected object, execute
   the unchanged `bin/cc` wrapper through `cmake -E env
   PSX_GCC=<verified-path>`; the unselected command branch stays textually and
   behaviorally canonical.
3. Make `compile_commands.py` emit the same selected-object environment so
   `flag-search` begins from the actual build profile. Configure dependencies
   must include the object-profile file and variant catalog.
4. Doctor validates only selected IDs: missing/mismatched selections are clear
   failures, never canonical fallback.

**Validation:**

```sh
just build
bin/asm-diff TARGET@0xADDRESS --detail full
bin/byte-match TARGET@0xADDRESS
bin/symbols check TARGET
bin/decomp-status TARGET --detail normal
just doctor
just check
```

Compare unselected object hashes with the Phase-0 manifest. Require fresh exact
matching for the selected function and matching compile-command/CMake selection.

**Acceptance:** with no `BOF3_OBJCOMPILER_` entries, every normal command and
unselected object is canonical; with one entry, only its object is variant-built
and it exact-matches.

**Rollback:** remove that one object entry and rebuild. Remove catalog data only
after no retained entry references it.

## Phase 4 — one bounded pilot and durable evidence

**Affected files:** update this plan; add/update
`docs/specs/runtime/compiler-variants.md` only for confirmed provenance and
concise results.

1. Revalidate one pilot’s source, boundary, load address, and live first diff.
2. Install/test the minimum Phase-0 candidates one at a time using Phase 2.
3. Promote only a clean-C exact result through Phase 3. Otherwise retain only a
   concise negative result tuple: compiler ID, identity, digest, flags, target,
   address, size/first diff, and stop reason.
4. Do not generalize a winning version to a target or duplicate group. Each
   object independently earns its profile.

**Acceptance:** every retained object profile is reproducible by a fresh exact
`bin/byte-match`; all broad checks above pass.

**Stop:** after the initial matrix is exhausted without exactness, or new source
/ ABI evidence says this is not a compiler residual, return to the normal lifting
ladder rather than add more variants.

## Ownership and open blockers

- The toolchain layer owns install, path, environment, identity, and digest
  verification. `bin/` wrappers only dispatch.
- Candidate archive/source licensing, immutable locations, checksums, host
  compatibility, and assembler compatibility are open research blockers. They
  must be answered before Phase 1 records a candidate.
- The canonical GCC archive’s observed local identity is not sufficient
  provenance for new candidates. Backfilling canonical archive verification is a
  separate improvement unless shared verification requires it without changing
  default behavior.
- No source/map/Splat/SDK ownership changes are part of this plan.
