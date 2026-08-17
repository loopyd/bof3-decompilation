# Documentation readability refresh

## Goal

Make root documentation and `docs/` accurate, approachable, easy to navigate,
and durable: remove run-specific/transitive narration, stale implementation
claims, duplicated guidance, broken navigation, and structure that obscures
the current repository contract. Preserve history, provenance, uncertainty,
active plans, and all safety/ownership/matching contracts.

## Scope and authority

- Root entry docs: `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`,
  `CREDITS.md`, `CODE_OF_CONDUCT.md`, `AGENTS.md`, `SOUL.md`.
- `docs/`: user guides, agent contracts, durable specs, imported EU
  reference, active plans.
- Two verified active-contract fixes in `.pi/skills/bof3-re/SKILL.md` and
  `.pi/skills/psx-rizin/references/DECOMP_BUILD_DIFF.md` (`--time-limit 300`
  → `60`, matching the live CLI; `agent-skill-compaction` audit and contract
  checks follow).
- Two explicit non-doc exceptions: the `psyq-bindings` help string in
  `tools/python/harness/commands/symbols.py`, and the catalog-vs-selection
  policy note in `config/compiler/variants.json`.

## Phases

### Phase 1 — audit and information architecture

- [x] Classified every root and `docs/**/*.md` file (72 files: 7 root + 65
      docs) by audience/authority: onboarding, contribution, usage, agent
      contract, durable spec, imported reference, history, or active plan.
- [x] Identified stale claims, broken links/anchors, duplicate prose,
      obsolete paths/commands, run-specific narration, and misplaced
      material with exact evidence, including live matching-state re-audits
      of every runtime/data-spec function claim and a systematic
      stale-occurrence scan.
- [x] Re-ran the complete link/anchor, stale generated-evidence,
      command/path, duplicate-prose, authority, and live matching-state
      audits across the full tree so same-pattern omissions are caught.
- [x] Defined the documentation journey: root `README.md` → `docs/index.md`
      → setup, usage, architecture, specs, contribution.
- [x] Preserved all active plans, historical changelog/reference content,
      and EU provenance headers verbatim.

### Phase 2 — root documentation

- [x] Rewrote `README.md` as a concise landing page: purpose, prerequisites
      (including host CMake), quick start, status commands, and docs map.
- [x] Brought `CONTRIBUTING.md` into a consistent structure; preserved
      `CHANGELOG.md` history byte-for-byte from `## 2026-07-31`; kept
      `CREDITS.md`/`CODE_OF_CONDUCT.md` stable.
- [x] Updated `AGENTS.md` only for verified drift (manifest-owned `psyq`
      source contract); preserved all ownership/matching/verification
      contracts.

### Phase 3 — navigation, guides, and active contracts

- [x] Rewrote `docs/index.md` as the canonical audience map.
- [x] Reconciled `docs/usage.md` and agent guides with current commands,
      paths, package structure, and ownership; removed duplicated procedures
      in favor of one authoritative owner and direct links.
- [x] Corrected the loaded `.pi` skill contract (`--time-limit 60`) and the
      compiler-candidate procedure order (catalog first, probe second,
      selection separate) in `docs/agents/matching.md` and
      `docs/agents/matching-playbook.md`; removed the stale
      `toolchains/README.md §20` citation and the malformed permuter
      sentence; reconciled `docs/agents/lessons.md` with the shared PsyQ
      SDK-space contract and removed its run receipt.
- [x] Kept the reverse/agents/review agent-context output at or below HEAD
      after compaction (see Validation).

### Phase 4 — specs, references, and transient cleanup

- [x] Audited every `docs/specs/**/*.md` claim against tracked evidence and
      owning commands; corrected stale generated-evidence citations
      (`out/index/vast-violence-1.1.json`, `out/reports/`, `out/catalog/`,
      `out/analysis/media/`) to real validators or tracked owners.
- [x] Corrected live matching-state claims: `exe/slus_004_22@0x80162B08`,
      `battle/15@0x800AF66C`, `GAME.EMI#0@0x80196FFC`, `0x80197378`,
      `exe/logo@0x801CE760`, `GAME.EMI#1@0x801D17D8`, and the
      `0x80165D48` dispatch are documented as live-exact (verified with
      `bin/asm-diff`/`bin/byte-match`); the `frontend.md` queue row was
      updated to reflect six tracked (3 exact, 3 partial) + eight unlifted.
- [x] Consolidated cross-file duplicate data tables (character equipability
      bitmask, shop-item reference) to one authority with a link; kept the
      deliberate per-chapter EU provenance header.
- [x] Recorded durable platform requirements (native audio needs x86-64-v4
      and `libFLAC.so.14`; conversion receipts must match manifest fields or
      be reproduced separately) instead of host receipts.
- [x] Preserved unknowns/limitations/provenance; narrowed the data-index
      frontmatter and `docs/index.md` summary to historical, provenance-
      scoped findings that lack a tracked byte-verifier.

### Phase 5 — plans and closure

- [x] Preserved all active/incomplete plans; this plan supersedes only the
      documentation-readability scope.
- [ ] Independent full-tree readability, factual-drift, navigation,
      link/anchor, command/path, duplication, and authority-boundary
      reviews: ran; same-pattern corrections applied; evidence recorded
      (see Validation results). Both policy gates now pass — `.pi`
      compactness 68,973 ≤ 69,000 and `test-skill-scripts.py` exit 0 —
      so this plan remains open only pending final independent
      acceptance of the restored `.pi` semantic qualifiers and the
      shared naming-debt baseline (`just check` stays red solely on
      that known `bin/symbols check` debt, byte-identical to HEAD).

## Validation

```sh
python3 .pi/skills/bof3-re/scripts/agent-context.py agents
python3 .pi/skills/bof3-re/scripts/test-skill-scripts.py
PYTHONPATH=tools/python .venv/bin/pytest -q tools/python/tests
.venv/bin/ruff check tools/python
bin/symbols check
just check
git diff --check
git diff --cached --quiet
```

Result (final closure review, 72-file tree; content work complete, both
policy gates pass, plan open only for final independent acceptance):

- Corrected GitHub-slug link/anchor audit: **73 fragment-bearing link
  occurrences, 52 distinct resolved (file, fragment) pairs, 0 broken files,
  0 unresolved anchors** (counting definition: every Markdown link carrying
  a `#fragment` in the 72 root+`docs/` files, resolved against GitHub slug
  rules including duplicate-heading suffixes; occurrences count the
  multiline link split across `docs/agents/lessons.md:109-110`;
  `(file, fragment)` pairs are the distinct resolved destination
  file+fragment; vendored `toolchains/` excluded).
- Cross-file nonblank duplicate-window scan (6–10 lines): **0 duplicate
  blocks** in `docs/specs/`; only deliberate per-chapter EU provenance
  headers remain identical.
- `bin/asm-diff`/`bin/byte-match` exact for all seven corrected selectors
  plus the documented compiler override at `exe/slus_004_22@0x8015DF18`
  (671/671, 2684 bytes).
- `docs/specs/targets.md` exactly matches all 23 target manifests; load
  addresses agree with manifest owners.
- `bin/str-media inspect`/`validate` reproduce the pinned CAPCOM30 SHA-256,
  1013 sectors, 203 frames, 37800 Hz stereo XA, 6.72 s, status pass; the
  conversion receipt correctly remains `status: fail`.
- All distinct current-facing `bin/*` names resolve to live wrappers except
  the intentional `bin/bof3-audio` corrective mention (documented as
  removed).
- Python suite: **414 passed / 0 failed** (measured 2026-08-16). Ruff
  clean. Scoped symbol checks pass.
- Agent-context output (battle/15, same generated asm inputs, measured
  2026-08-16): full selector payloads reverse 99,960 < 100,000, review
  97,827 < 100,000, agents 71,728; static-prefix contexts (before
  selector evidence) reverse 77,432 / review 75,299 — `test-skill-scripts.py`
  passes (exit 0).
- Conversion-manifest contract corrected: the generated `conversion.json`
  records the source hash as the nested `validation.source_sha256` field,
  verified against the producer (`media/str_media.py`) and the live
  CAPCOM30 manifest; no top-level `source_sha256` claim remains.
- Schema-ledger authority aligned: the `storage` status meaning now uses the
  `storage-verified` vocabulary (recorded structural provenance, no tracked
  byte-verifier re-check) matching `ids.md`'s Evidence boundary.
- `git diff --check` clean; staged index empty; submodule gitlinks
  unchanged; no tracked diff under `inputs/`, `out/`, `build/`, or
  `toolchains/`.

### Final closure corrections (independent review)

- The Phase 5 closure-review checkbox no longer claims completion: the plan
  explicitly stays OPEN while the two baseline-red policy gates under
  Blockers keep `just check` and `test-skill-scripts.py` red.
- `docs/index.md` now lists `documentation-readability-refresh.md` and
  `harness-package-refactor.md` under Plans as open (content complete;
  final independent closure review pending on shared required gates), not
  under completed historical implementation records.
- `docs/specs/data/schema-ledger.md` GAME.EMI layout section now uses the
  `storage-verified` vocabulary (recorded structural provenance, no tracked
  byte-verifier re-check) instead of claiming byte-exact storage maps; the
  attestation matches the `storage` status meaning and `ids.md`'s Evidence
  boundary.

## Blockers and residual risks

- **Out-of-scope drift:** `toolchains/README.md:39-41` still says only two
  GCC candidates exist with no object selection; live catalog lists four and
  `config/compiler/object-flags.cmake:61` selects `gcc-2.6.3-psx` for exact
  `dispatchSoundCue`. This plan forbids editing `toolchains/` docs; a
  separately authorized owner fix is required.
- **Native audio behavior is environment-blocked here:** the tracked ELF
  requires x86-64-v4 and `libFLAC.so.14`; this host reaches neither `main`
  nor `--help`. Source/`readelf` verification supports the documented
  contract; no native behavior acceptance is available.
- **Policy gates pass (measured 2026-08-16).** The `.pi` compactness gate
  passes: 68,973 bytes ≤ 69,000
  (`test_agent_and_skill_context_files_stay_compact` green).
  `test-skill-scripts.py` passes exit 0; full selector payloads for
  `emi/battle/battle/15@0x80096E90` are reverse 99,960 / review 97,827 /
  agents 71,728 (all < 100,000). Full Python suite: 414 passed. The
  required `agent-skill-compaction` audit (`--check` against the pre-edit
  baseline) reports zero errors and zero grown files. The `.pi` semantic
  qualifiers flagged by final review were restored from HEAD; equivalent
  contract-safe terse bytes were found elsewhere in `.pi`/`docs` to keep
  both gates green.
- Global `bin/symbols check` naming debt is byte-identical to HEAD
  (SHA-256 `81a188db711ea8f078f0d0bd5bc421ea212b432f38b316bfa7d1fc6401cfe895`).
- Ignored private/generated state exists (licensed media, `out/`, `build/`,
  toolchains, caches); none is staged or tracked by this diff.

## Boundaries and non-goals

- No edits to source, configuration, maps, Splat, binaries,
  generated/private inputs, or `toolchains/` docs beyond the two explicit
  exceptions above.
- No staging, commits, pushes, or external mutations.
- No removal of valid historical records; no weakening of agent contracts,
  evidence rules, safety gates, ownership, or validation contracts.
- No license inference: the repository has no `LICENSE` (commit `fcca1546`
  removed it); `CREDITS.md` third-party notices are not a license.
- Prefer deletion, direct prose, short sections, tables only where
  scannability improves, and one authoritative page per procedure.
