# AGENTS.md — BOF3

First repository command: `bin/agent-context ROLE` (plus the prompted selector
for reverse/review/cleanup roles). The active role definition owns the exact
invocation; use `agents` only when no narrower role is active. Run it once. Its
bounded, tracked output owns canonical reading order; do not rerun it or reread
an emitted path without a named evidence gap. Use repository-relative commands
without `cd`.

After `.pi` agent/skill Markdown edits, run `/skill:agent-skill-compaction`.

BOF3 binaries load independently. Qualify work by one function selector:
`TARGET@0xADDRESS`. For a shipped EMI entry, use
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`.

## Boundaries

- Work only in this repository; never commit user media from `inputs/`.
- `out/` is disposable working state — edit candidates there only when a
  tool's workflow requires it, never treat it as reviewed truth or commit it.
- Do not hand-edit `build/` or `toolchains/`.
- Analyze an executable image or extracted EMI entry, never an EMI archive;
  keep identical addresses or bytes in different targets as separate facts.
- Original bytes and PS-X headers outrank analyzer output. Verify `t_addr`.

## Ownership

| Fact                             | Owner                          |
| -------------------------------- | ------------------------------ |
| Binary identity and load address | `config/targets/<target>/target.toml` |
| Reviewed layout                  | `config/targets/<target>/splat.yaml`                |
| Target-local symbols             | `config/targets/<target>/symbols.txt`  |
| Shared SDK symbol maps           | `config/sdk/psyq-{slus,logo}.txt` |
| Reviewed Rizin annotations       | `config/targets/<target>/reviewed.rz`    |
| Authored lifts                   | `src/bof3/<subsystem>/`         |

## Source and symbols

- Keep one C source per lifted function under `src/bof3/<subsystem>/`. Lift identity and target ownership come from explicit manifest claims, maps, Splat, and parsable function-level `@source`/`@behavior` metadata—never directory ancestry or filenames.
- Use `func_80143B40`/`D_80143B40`; maps use sorted `name = 0xADDRESS;`
  entries with eight uppercase hex digits.
- Replace raw names only after review; a renamed lift file keeps
  `@behavior`/`@source` metadata (the `@source` tag is the address authority).
  Resolve name collisions with a different name or a suffix
  (`D_80146864_BYTE`), never an overlay-name prefix (`SCENA16_D_*`). Every
  non-address-named map symbol (SDK exempt) carries its origin address in a
  `/* @source 0xXXXXXXXX */`-tagged definition (lift file, declaration, or
  `WEAK_SYMBOL_AT` binding); `bin/symbols check` enforces both rules.
- Edit the metadata-resolved lift source, its target `internal.h`, the target-local map, and reviewed Splat boundaries as evidence improves. Source filename, compiled symbol name, and Splat label are separate identities tied by metadata and target-local address. Keep declarations local unless a demonstrated cross-target contract requires sharing.
- Keep PsyQ external: use official declarations and the shared SDK symbol maps;
  never lift its bodies. The PsyQ/BIOS runtime is a shared SDK linked into the
  main exe (`SLUS_004.22`); every EMI overlay calls those functions at the same
  fixed addresses, so they share one `slus` SDK space
  (`config/sdk/psyq-slus.txt`). `LOGO.EXE` owns a distinct `logo` space
  (`config/sdk/psyq-logo.txt`); a target selects its space via the manifest
  `[psyq] space` key (default `slus`). This cross-target reuse is authorized
  by the pinned SDK version (the `toolchains/psyq/4.7` include path in
  `CMakeLists.txt` and `docs/specs/runtime/psyq-constants.md`), not by
  coinciding game bytes. Treat the SDK maps as a switchable weak-binding layer
  (`WEAK_SYMBOL_AT`) that a real SDK library can later override one symbol at a
  time.
- The compiled PsyQ source is manifest-owned: `bin/symbols psyq-bindings`
  writes `manifest.psyq_source` (e.g. `src/bof3/support/slus_psyq.c`) from the
  SDK map, tracked only because the build compiles it (CMake globs `src/*.c`);
  regenerate; never hand-edit. The full-composed
  bindings under `out/bindings/` (regenerated on every match) stay disposable
  and untracked.
- Write readable C89. Inline assembly is banned in lifted source except the
  sanctioned helpers: `barrier()`/`CLOBBER_*` (`include/base/barrier.h`) for
  access ordering and delay-slot placement, `REGISTER_PIN(type, name, reg)`
  (`include/base/barrier.h`) for an approved allocator constraint, and
  `WEAK_SYMBOL_AT` (target's manifest-claimed `symbols.c` only) for address
  binding. Do not use direct `register X asm("$N")` pins, `extern X asm("NAME")`
  symbol renames, or handwritten assembly; only retain a direct
  numeric-register spelling when the macro form has been demonstrated to change
  codegen. Bind symbols with a plain `extern` in `internal.h` plus a
  `WEAK_SYMBOL_AT` entry in that `symbols.c`. After the clean-C matching ladder
  is exhausted, an asm-diff-proven allocator or entry-register residual may
  make one bounded local `REGISTER_PIN` experiment without additional approval;
  retain it only with an adjacent `MATCHING_AID` comment, a live byte-match,
  and independent review. A direct numeric spelling still needs explicit user
  approval; `INCLUDE_ASM` still needs explicit user approval.

## Exact duplicates

- Treat `(analyzer-range SHA-256, size)` as a reuse candidate, not shared
  ownership. Confirm reviewed boundaries, then iterate on one representative.
- A tracked or partial lift is not a reusable implementation; use its match
  percentage only to prioritize the next edit, never to promote other members.
- Port the representative C shape to a second member only after the first
  byte-matches; keep both sources independent until both byte-match.
- Normalize proven roles, parameters, local names, struct names, and field
  names across group members; keep unknown fields offset-based.
- After two cross-target members independently byte-match with the same C shape, move only a worthwhile common body to `src/shared/<domain>/<role>.inc`. Keep one metadata-tagged target-local wrapper per member to provide its compiled symbol and any compile-time parameters; its filename may be semantic.
- Put stable shared types in `include/<subsystem>/` (e.g. `include/battle/`,
  `include/gpu/`).
  A shared template is compiled into every owning image; it is not a runtime
  engine service.
- Never reuse a game-specific extern address across targets. Each wrapper
  retains its target-local map, declaration, Splat boundary, and independent
  `asm-diff`/`byte-match` validation. The shared PsyQ/BIOS runtime is exempt
  (see above).

## Completion

- See every assigned task through: do not skip requested steps, defer
  subtasks, or stop at partial results. Finish the work, run the checks, and
  report residual risks.
- Stop early only on an evidence-backed blocker: name the blocker, what you
  tried, and the smallest unblocking action. Never silently drop scope.

## Verification

- Tests verify behavior or parsed structure, never literal wording or source
  substrings from agent/skill/prompt/workflow Markdown; no brittle
  content-lock tests for policy text.
- `bin/asm-diff` = instruction evidence; `bin/byte-match` = bytes.
- `bin/symbols check` after map edits; normalize with
  `bin/symbols normalize [TARGET] --write`. Global checks gate naming debt
  against `config/symbol-naming-baseline.json`.
- `bin/decomp-status [TARGET...]` = live lift audit.
- `just check` before handoff when practical; list skipped checks.
- No stage/commit/push/external mutation without approval.

Load `/skill:bof3-re` for ANY lifting, matching, duplicate-normalization, or
promotion task — it enforces the [memory API](docs/agents/memory-api.md)
inline-assembly ban and the [function-matching](docs/agents/matching.md) loop.
Classify each live first mismatch with the [matching playbook](docs/agents/matching-playbook.md):
no allocator pin for a frame/size or CFG mismatch, no clobber until a
caller-register scheduling placement is proven. The ignored
`out/non-exact-lifts.json` audit is parent-generated priority state, not
evidence that replaces a function's live `asm-diff`.
Use `/skill:psx-rizin` only for explicitly requested generic analyzer work.
See the [docs index](docs/index.md), [tool usage](docs/usage.md), and the
[repository map](docs/agents/project-context.md#repository-map). Store reviewed
findings in `docs/specs/` and gotchas in `docs/agents/lessons.md`.


## Planning

For repository implementation plans or plan-management requests, read
[`docs/agents/plan-authoring.md`](docs/agents/plan-authoring.md), then create or update a scoped plan under
`docs/plans/`. Keep plans phased, evidence-backed, and aligned with live
`bin/decomp-status`, `bin/symbols check`, and validation results; durable
runtime or format findings still belong in `docs/specs/`.
