# Managed historical GCC variants

**Status:** Phases 0–2, 4, 6, and 7 are complete; no other active local
implementation phase remains. Phase 3/5 object-selection plumbing is
implemented but **cannot be marked complete**: no target has a clean-C, fresh
exact `BOF3_OBJCOMPILER_` profile. The concrete trigger to close it is a new,
independently evidenced candidate that fresh `bin/byte-match` exact-matches a
live target with a retained object selection — not a fake selection/flag
profile and not a reopened pass over the negative pilot. Phase 4 tested its one
provenance-pinned candidate and is **closed
negative**: `gcc-2.6.3-psx` did not exact-match its bounded pilot, so no
object profile is retained.

**Scope:** add an opt-in, provenance-pinned way to compare one lifted function
against managed historical PS1 GCC variants. The canonical compiler remains
`gcc-2.7.2-psx -> maspsx -> ASPSX 2.56 emulation`.

## Goal and evidence baseline

`emi/battle/battle/15@0x800AF66C` exhausted its readable-C, supported-flag,
permuter, and bounded local-pin ladders. Its disposable clean-C pilot was
same-size but had an entry-register allocation mismatch. The original overlay
contains no debug/compiler provenance strings, so its compiler version and
options remain unproven. The one `gcc-2.6.3-psx` comparison is recorded below
as negative evidence; neither pilot source nor compiler profile is retained.

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
- Do not change `bin/cc`, the default compiler selection, or normal build
  commands when no object selects a variant. `just setup` may prime
  provenance-pinned host-compatible catalog archives/installs, but never sets
  `PSX_GCC` or selects a compiler for an object.
- Do not add a generic compiler wrapper, target-wide compiler setting, an
  unbounded research sweep, or a matching macro. Selected-profile recovery may
  install only the requested catalog ID from its digest-verified cache.
- Do not retain a compiler profile from score, assembly resemblance, or raw
  object similarity: the specified `TARGET@0xADDRESS` must pass fresh
  `bin/byte-match` with clean C.
- A catalog record requires an authoritative identity/source, license status,
  immutable archive, SHA-256, expected executable
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

2. For each usable candidate, collect the source/release identifier and exact
   immutable archive URL, SHA-256, license/notice, host requirements,
   executable-relative path, `--version` expectation, and evidence it works with
   current `-gcoff`, MIPS flags, maspsx, and ASPSX 2.56 emulation.
3. Capture a disposable normal-build control in `out/`: canonical compiler
   identity, `just build` result, and a sorted SHA-256 manifest of built objects.
   This is comparison evidence only and remains untracked.
4. Historical pilot prerequisite: `0x800AF66C` had no maintained source at
   planning time. Its later disposable pilot is closed negative in Phase 4; do
   not reuse a cached permuter artifact for any future experiment.

**Acceptance:** all proposed candidate facts are independently sourced; the
canonical control validates; no candidate lacking provenance advances.

**Stop:** if no safe, compatible, verifiable candidate exists, publish the
negative evidence and stop without code changes.

**Decision (2026-07-23):** Initial research found no candidate with enough
provenance to catalog. The later `gcc-2.6.3-psx` record below met that bar and
was tested once; it did not produce an exact pilot match. The default toolchain
remains unchanged.

## Phase 1 — catalog and managed variant lifecycle

**Affected files:**

- add `config/compiler/variants.json`;
- update `.gitignore` and `toolchains/README.md`;
- update `tools/python/harness/io.py`;
- add candidate-only lifecycle code under `tools/python/harness/toolchain/`;
- add focused tests under `tools/python/tests/`.

1. Add a strict, versioned catalog for non-default archive compilers. Each safe
   unique ID maps to its Phase-0 provenance, archive digest,
   executable-relative path, expected identity, host constraint, license notice,
   and assembler assumptions.
2. Give `RepoLayout` ignored install roots:
   `toolchains/gcc-variants/<id>/` for variants and
   `toolchains/gcc-2.7.2-psx/` for canonical GCC. GCC archives are cached under
   `inputs/external/private-assets/toolchains/gcc/`; unrelated toolchain
   downloads remain in `toolchains/downloads/`.
3. Implement archive-candidate resolution, SHA-256 verification, path-safe
   extraction, executable/identity verification, and safe-path checks in the
   Python managed-toolchain layer. Fail closed before execution on an unknown
   ID, traversal path, bad digest, wrong identity, or unsupported host.
4. Add one thin `bin/compiler-variants` dispatcher with only:
   `list`, `install <id>`, `verify <id>`, and `path <id>`. `just setup` primes
   every host-compatible catalog entry; doctor remains non-mutating and
   verifies only selected IDs.

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
   an unselected source, invoke the existing compile command via `bin/cc` with
   `PSX_GCC` set only in that subprocess environment; preserve the existing
   temporary output and relocation-aware comparison path. For a command that
   already embeds `cmake -E env PSX_GCC=...`, strip that one embedded assignment
   so the explicit subprocess environment replaces the retained selection.
3. Emit the requested candidate ID and label (when selected), flags,
   byte-match status, and instruction percentage. The command must not download
   implicitly, write `object-flags.cmake`, change CMake configuration, or mutate
   `build/`. The catalog—not flag-search output—remains the authority for
   identity and archive digest.
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

**Status: COMPLETE — NEGATIVE** (2026-07-31). The one-candidate matrix is
exhausted; do not expand it from score/assembly resemblance.

| Field | Verified evidence |
| --- | --- |
| Candidate | `gcc-2.6.3-psx`, old-gcc release `0.13` commit `b9793e7e84f42d442e8d89a2c5c9e568e79e3bb7` |
| Archive | `https://github.com/decompals/old-gcc/releases/download/0.13/gcc-2.6.3-psx.tar.gz` |
| Digest / host / identity | `sha256:db98510a8cece2f9e37665cc16b4f1f7ad17f282f900d2791b62ed74f50e40b2`; `linux-x86_64`; `gcc --version` = `2.6.3` |
| Pilot | Disposable `emi/battle/battle/15@0x800AF66C` clean-C candidate, 76 bytes, reviewed boundary/load/payload offset `0x18E6C`; removed after non-exact closeout |
| Compatibility / result | A temporary C89-compatible spelling (`void func_8009B20C(void) __attribute__((noinline));`) let 2.6.3 compile the pilot through `bin/cc → maspsx --aspsx-version=2.56 → bin/as`; it was restored before closeout. Of 52 flag profiles, 47 compiled and all differed; best was 19.05% (`-O1 -fno-delayed-branch`), while 5 unsupported `-mno-split-addresses`/`-Os` profiles failed to compile. No object override was added. Canonical live control remains 76→76 bytes, first `+0x0000`: original `move t0,a1; move v0,zero`, current `move a2,a1; srl a3,a2,1` (2/19 instructions). |
| Stop reason | No clean-C exact `bin/byte-match`; no compiler or flag profile may be retained. |

The catalog record remains as provenance-pinned, opt-in negative evidence; its
installation state is ignored. Remove it only if it becomes obsolete or its
source/archive provenance is withdrawn. A future experiment needs new source,
ABI, or compiler provenance evidence—not another pass over this pilot.

## Phase 5 — retained per-function compiler specification metadata

**Status:** metadata contract implemented; selected-object validation pending
a fresh exact profile (see the header trigger).

**Prerequisite:** Phases 1–3 infrastructure present. No pilot required to
implement or test the metadata/override contract; retaining a profile still
requires Phase 4's clean-C exact-match gate.

**Affected files:** `config/compiler/object-flags.cmake` (comments and
entries, schema unchanged); `CMakeLists.txt` (no change — plumbing exists);
`tools/python/harness/compiler_config.py` (no change — parser exists);
`tools/python/harness/commands/compile_commands.py` (no change —
variant env emission exists); `tools/python/harness/commands/doctor.py`
(no change — variant inspection exists); `docs/specs/runtime/compiler-variants.md`
(concise evidence records only).

This phase defines the standard for recording per-function compiler metadata
when a nondefault compiler and/or flags are proven necessary. It does not add
new infrastructure — it codifies how to use the existing
`BOF3_OBJCOMPILER_<key> <catalog-id>` and `BOF3_OBJFLAGS_<key> <flags>`
entries in `config/compiler/object-flags.cmake`.

### Metadata specification

The standard applies when a `BOF3_OBJCOMPILER_` or `BOF3_OBJFLAGS_`
entry is added or changed; it does not retroactively assert evidence for
existing reviewed flag exceptions.

1. **Target-qualified evidence comment.** Precede the changed `set(...)` entry
   with `TARGET@0xADDRESS`, the exact flags, and the fresh
   `bin/byte-match TARGET@0xADDRESS` command that passed. For a nondefault
   compiler, also name its catalog ID. The catalog entry, rather than copied
   `--version` output or an executable hash, owns the identity and archive
   checksum evidence.
2. **Default rule:** canonical `gcc-2.7.2-psx` + canonical flags need no entry.
   An entry exists only when a function demonstrably requires a nondefault
   compiler and/or nondefault flags. A `BOF3_OBJCOMPILER_` entry without a
   companion `BOF3_OBJFLAGS_` entry means the function uses the canonical flags
   but the nondefault compiler.
3. **Proven-only retention:** retain a compiler version only after clean C
   passes `bin/byte-match`. Retain a flag exception only after the same gate.
   No score percentages, near-match evidence, or assembly-resemblance profiles
   are retained. The negative-evidence record belongs in
   `docs/specs/runtime/compiler-variants.md`.

### Validation and parity

The following tools must use the same retained compiler profile for a given
function. No tool adds a variant path the others do not recognize:

| Tool | Role |
| --- | --- |
| `CMakeLists.txt` | Resolves `_variant_id` from `BOF3_OBJCOMPILER_<key>`; wraps
  compile in `cmake -E env PSX_GCC=<verified-path>` |
| `bin/cc` | Receives `PSX_GCC` from CMake environment; dispatches the specified
  compiler unchanged |
| `bin/flag-search` | With no `--compiler`, uses the compile-command selection.
  With `--compiler <catalog-id>`, removes an embedded `cmake -E env
  PSX_GCC=...` assignment before compiling, so the explicit catalog ID replaces
  the retained selection |
| `compile_commands.py` | Reads `BOF3_OBJCOMPILER_<key>` and emits `PSX_GCC` in
  `compile_commands.json`; its flags and compiler selection must mirror CMake |
| `doctor.py` | Inspects `BOF3_OBJCOMPILER_` selections and verifies every
  referenced variant ID is installed and valid; it does not validate flags |

### Completed override implementation

`tools/python/harness/match/flag_search.py` now removes an embedded
`cmake -E env PSX_GCC=...` assignment only when `--compiler <id>` is explicit;
the requested verified variant then reaches `bin/cc` through the subprocess
environment. Focused tests cover removal, preservation of other CMake
environment assignments, and unchanged commands without the embedded setting.
No-option behavior remains unchanged.

### Code-maintenance checks

1. After adding or modifying any `BOF3_OBJCOMPILER_` or `BOF3_OBJFLAGS_` entry:
   - Run `just build` — must succeed
   - Run `bin/byte-match TARGET@0xADDRESS` — must pass fresh exact match
   - Run `just doctor` — must not report a selected-variant failure
   - Run `bin/symbols check TARGET` — must pass
2. After adding a new compiler catalog entry:
   - Run `bin/compiler-variants list` — new ID appears
   - Run `bin/compiler-variants verify <id>` — passes
3. After any metadata change, run `just check` before promotion.

### Config dependencies

- `config/compiler/variants.json` — the single source of truth for compiler
  catalog IDs, digests, and executable paths. The build does not reference any
  compiler ID not present in this catalog.
- `config/compiler/object-flags.cmake` — the single source of truth for
  per-object compiler and flag overrides. No other file holds this mapping.
- `config/compiler/flag-catalog.json` — read by `bin/flag-search` for flag
  profile enumeration; not read by the build system directly.
- `compile_commands.json` (ignored, generated) — emitted by
  `compile_commands.py` from the above two config files.

### Rollback

Remove the `BOF3_OBJCOMPILER_<key>` and/or `BOF3_OBJFLAGS_<key>` entry from
`config/compiler/object-flags.cmake`. Rebuild. Remove catalog data from
`config/compiler/variants.json` only after no retained entry references it.

**Acceptance:** with no `BOF3_OBJCOMPILER_` entries, every command and
unselected object is canonical; with one entry, only its object is
variant-built and it exact-matches. All parity checks above pass.

## Phase 6 — validate the GCC → maspsx → linker match path

**Status: COMPLETE** (2026-07-30)

**Goal:** prove that every currently exercised compiler-selection path used by
lifting and matching (canonical `bin/cc` and explicit subprocess `PSX_GCC`)
still passes through `bin/cc`'s GCC assembly, maspsx translation, ASPSX
assembly, and relocation-aware linker comparison. Catalog-selected
CMake/`compile_commands.json`/doctor paths remain untested for a selected
object because no object selects the available candidate
(`gcc-2.6.3-psx`), not because no candidate exists; they are not live-tested
here. This is a
toolchain contract check, not a new matching macro or an alternate linker.

**Evidence baseline:** `bin/cc` compiles C to `.s`, pipes it through
`third_party/maspsx/maspsx.py` with the selected ASPSX version and optional
`--expand-div`, then calls `bin/as`; `bin/ld` is the PSn00b GNU linker. The
matching harness builds through CMake, then links the resulting object at the
function address with the PSn00b linker before comparing bytes.

**Affected files:** focused harness tests under `tools/python/tests/`; the
smallest applicable `bin/cc`/matching-harness code only if testing exposes a
real contract gap; `.pi/agents/bof3-reverse.md`, `.pi/agents/bof3-review.md`,
and `.pi/skills/bof3-re/SKILL.md` for the retained agent gate; this plan and
`docs/specs/runtime/compiler-variants.md` for proven results. No alternate
compiler driver, linker, or source-level matching aid was added.

### Completed tasks

1. ✅ **Hermetic `bin/cc` stub test**
   (`tools/python/tests/test_bin_cc_pipeline.py`) — 3 tests using executable
   stubs that record arguments. Verifies GCC receives `-S`, maspsx receives
   `--aspsx-version=2.56`, `-Wa,--expand-div` reaches maspsx but is absent
   from final assembler. All passing.

2. ✅ **Relocation-aware comparison test**
   (`tools/python/tests/test_asm_link.py`) — 4 fixture-local tests using real
   PSn00b binutils (no game inputs). Assembles minimal MIPS assembly, links at
   `0x80010000`, extracts bytes. Round-trips through harness API. All passing
   (skipped when PSn00b toolchain not installed).

3. ✅ **Live control tests** — exact lifts verified intact:
   - `emi/etc/game/01@0x801D0D5C` — MATCH (canonical `-O1`)
   - `emi/battle/battle/15@0x800AB760` — MATCH 53/53 (`-O2 -Wa,--expand-div`)
   - `emi/battle/battle/03@0x801E29B4` — MATCH (`-O2 -Wa,--expand-div`)
   These three controls use the canonical compiler with reviewed
   `BOF3_OBJFLAGS` overrides only; no catalog-selected object profile was
   retained.
   - Tool versions recorded in `docs/specs/runtime/compiler-variants.md`

4. ✅ **Agent/skill pipeline-test contract** — narrow gate added to
   `.pi/agents/bof3-reverse.md`, `.pi/agents/bof3-review.md`, and
   `.pi/skills/bof3-re/SKILL.md`. Source-only lifts exempt.

5. ✅ **Plan status updated** — this section

### Acceptance criteria met

- Canonical and `PSX_GCC`-selected compilations use maspsx with ASPSX 2.56
- `--expand-div` preserved: reaches maspsx, absent from assembler
- Byte-match path links at function address before extraction
- Live exact lifts remain 100% byte-identical
- Focused tests pass
- `just doctor` passes

### Stop conditions NOT triggered

No focused test or live control diverged. No compiler metadata was broadened.
No broken boundary was recorded.

## Phase 7 — external GCC archive cache and selected-install recovery

**Status: COMPLETE** (2026-08-01).

1. Give `RepoLayout` one derived GCC cache root under `private_assets_dir`.
   Canonical GCC and every catalog variant use it; unrelated toolchain downloads
   stay in `toolchains/downloads/`.
2. Make GCC archive handling fail closed and recoverable: reject a symlink or
   non-regular cache entry, download into a cache-local temporary file, verify
   the catalog/canonical SHA-256 before atomically publishing it, and safely
   extract to a sibling staging directory. Verify the staged executable/version
   before atomically replacing a destination. A failed download, digest,
   extraction, or identity check preserves a prior working install.
3. `bin/compiler-variants path <id>` and generated `compile_commands.json`
   use the same ensure-installed operation. A selected compiler may download
   only its own catalog ID; unknown/unsupported IDs fail, never falling back to
   canonical or host GCC. Doctor remains verification-only.
4. `just setup` primes canonical GCC plus every host-compatible catalog entry
   in `config/compiler/variants.json`; host-incompatible candidates are
   skipped with their ID and host reported, and an invalid catalog fails
   setup closed. Setup never sets `PSX_GCC`, adds an object override, or
   changes the default compilation selection; catalog entries simply make
   every project-confirmed compiler version available on demand.
5. Add hermetic coverage for valid-cache recovery, corrupt/download-failure
   cleanup, atomic-install preservation, cache-symlink rejection,
   catalog-wide setup priming, and compile-command/path parity. Update the
   toolchain plan/spec/README and ignore rules with the final cache contract.

**Validation (2026-08-01):** focused GCC variant/setup/doctor/compile-command
tests (64 in `test_gcc_variants.py`, 5 in `test_setup.py`); live
`bin/compiler-variants path gcc-2.6.3-psx` after deleting only its ignored
install (auto-installed from the digest-verified cache); corrupt install fails
closed (`path` exit 2 with a repair hint, no canonical/host fallback); `just
setup` twice (8/8 tasks, idempotent prime — canonical archive cached with
`aca64479…`); `just doctor` 5/5; `just check` (243 tests, ruff, symbols,
validate_sources); `git diff --check` clean. `just setup` reported `gcc-2.6.3-psx` primed from
   the catalog even though no `BOF3_OBJCOMPILER_` selection exists; an empty
   catalog installs no variant, and an invalid catalog fails setup closed.

**Acceptance:** archive and install recovery are atomic and digest-verified;
all selected launch paths resolve the same verified compiler; default/no-
selection behavior remains canonical; no catalog research candidate is
implicitly activated.

## Ownership and open blockers

- The toolchain layer owns install, path, environment, identity, and digest
  verification. `bin/` wrappers only dispatch.
- Future candidate archive licensing, immutable locations, checksums, host
  compatibility, and assembler compatibility must be independently evidenced
  before a catalog record is added.
- The canonical GCC archive’s observed local identity is not sufficient
  provenance for new candidates. Backfilling canonical archive verification is a
  separate improvement unless shared verification requires it without changing
  default behavior.
- No source/map/Splat/SDK ownership changes are part of this plan.
