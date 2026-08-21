# Cleanup rules

Never turn audit into edits, docs repair into source refactor, or naming into lifting/matching. Cosmetic/evidence-preserving only; source filename changes require a metadata-backed naming transaction. Never change behavior/byte-match.

## Evidence gate

Retain names only with target-local address/layout and two independent corroborators: two consistent local accesses/calls; one + Rizin annotation; or proven layout/dispatch table + consistent use. Decompiler names, duplicate hashes, strings, comments, single sites, `data-scan` counts are leads. Keep `D_XXXXXXXX`/`unk_XX`/`field_XX` if uncertain. Name data by proven content class: strings, pointer/handler table, or struct layout + consumers.

Under two corroborators is not a ceiling:

1. Before semantics run `bin/naming-audit prepare TARGET`. A `safe_metadata_repair` requires a `bin/naming-audit prepare TARGET --repair` run, whose live `asm-diff`/`byte-match` proves exactness before canonicalizing progress metadata; ownership/layout stays blocked. Then run row inventory: `bin/rev-query --json inventory TARGET`; emit `bof3.naming-audit/v3`. Validate via `bin/naming-audit validate TARGET REPORT.json --transaction KIND:NAME`; other blocks do not. Afterward run `bin/naming-audit verify TARGET REPORT.json --transaction KIND:NAME` to prove scope, old-name absence, storage. `rev-query describe` owns payload/file/storage; `rev-query transaction-scope TARGET SYMBOL` owns transaction files.
2. Run `bin/rz-project status TARGET --json` and `bin/rev-query --json status`. If either is stale/unavailable, `bin/index --recover` reanalyzes **every** stale manifest snapshot then atomically rebuilds disposable `out/index/`; recheck both, never one target only. Recovery touches only `out/reverse/`/`out/index/`. Failure blocks; name target/command/smallest repair.
3. PS-X repair: original header outranks tools. Verify magic, `t_addr == manifest.load_address`, `t_size == payload size`, file offset `0x800`, `runtime - load == payload offset`; never analyze headers as payload or force mismatched maps. Malformed/truncated/mismatched images block the row; repair manifest/input/extraction, then recover/recheck. Raw overlays use offset `0`, never this formula.
4. When fresh, query `rev-query` calls/xrefs/symbols first; `rev-query xrefs TARGET@ADDRESS` supplies indexed data source/function/access kind/opcode. Reproducible bounded observations use `bin/rz-project query TARGET -c 'COMMAND'`; `open` is interactive only. Prove binding eligibility before semantics.
5. A function outside payload/boundaries is an import lead, not owned: run `bin/rev-query --json owners TARGET@0xADDRESS`, inspect every plausible candidate, then prove owner via manifest load/range, original bytes, boundary, runtime composition. Query calls/xrefs/variables/metrics; inspect proven body with `bin/rz-project open OWNER`. `reviewed_range`/`analyzer_range`/`mapped_entry` provenance/confidence/containment are leads. Propose only in its owner contract; SDK exceptions remain explicit. Same addresses across unrelated overlays are separate; a resident executable may own a shared service.
6. Outside data ownership uses maps/Splat/load ranges plus original-byte access/initializer/consumer proof, not function `owners`; only shared fixed-RAM uses the data exception below.
7. Empty/incomplete xrefs or owners require bounded analysis per proven image: candidate/delay slots, `jal`, aligned pointer-table entries, neighboring handlers/state accesses, original table bytes. A machine-code caller, callee body/state effects, dispatch layout, or state-machine consumer corroborates only with proven address model/boundaries. Analyzer/decompiler output is hypothesis; audit never mutates annotations.
8. Before `no-change`, go one semantic level beyond: table consumer/selector/neighbors; caller guards/transitions/result/stable arguments; transform/copy helper callers and source/destination roles; presentation helper callee + caller context; raw data's other consumer or initializer/use pair. Repeated unexplained shapes count once. Name the missing static fact. Runtime is optional when original-byte/layout/caller/consumer evidence suffices.
9. A semantic partial automatically passes only to rung-4 spelling, preserving body, ABI, address, boundary, compiler settings, `@status partial`, `@match`, `@residual`; exactness is unnecessary and matching edits cannot be bundled.

### Audit evidence

Every raw inventory row appears once, proposed or unresolved. Each retained/proposed rename has `rename_evidence`:

- identity: selector, function/data kind, old/new, unchanged address/range; binding/source locations exactly equal `rev-query transaction-scope`—omit no caller/manifest and invent no binding;
- function: Rizin commands plus callsite/instructions/delay slot/arguments/guards/result/transition; data: access/initializer/consumer commands plus instructions/layout;
- runtime owner: outside function `owners` plus manifest/load-range, bytes, boundary, composition; outside data maps/Splat/load ranges plus byte/use proof. Local may use `N/A — image owns range` only with range proof;
- owner function: commands plus callee instructions/delay slots/globals/tables/callees/consumers; owner data: accesses, initializer/consumers, class/layout;
--ID typed observations; corroborators map IDs to distinct mechanisms (duplicate range/shape/mechanism counts once); `name_terms` maps every semantic word to IDs, else narrow/reject;
- partial: live status/match/residual plus original-byte Rizin verification; C alone never counts;
- rejection: missing static fact plus next bounded Rizin/original-byte command.

Every command executes and records passed/failed status, a repo-relative `out/reviews/evidence/` receipt, and verified SHA-256; never fabricate. Also record observed instructions/bytes/addresses/effects: commands alone are not evidence.

| Kind/locality | Required typed `rungs` |
|---|---|
| local function | `selected_range`, `selected_call`, `one_level_beyond` |
| imported function | `selected_call`, `owner_resolution`, `owner_body`, `one_level_beyond` |
| local data | `selected_range`, `selected_access`, `storage_class`, `one_level_beyond` |
| outside data | `selected_access`, `owner_resolution`, `owner_data`, `storage_class`, `one_level_beyond` |

Partial evidence requires `partial_baseline`. Tool-generated `required_work` covers indexed callers/callees/accesses/owners; each item with commands + typed observations, evidence-deduplicate it, or leave it open. Concrete discoveries expand this bounded graph. `exhausted` requires all closed; open/failed work means `blocked`, never optional ceiling work. `optional_work`/`ceiling_next_command` hold post-ceiling experiments only. Proposals record `semantic_status`, `transaction_status: ready|repairable|blocked`, `readiness_blockers`; proposed data storage exactly equals `describe`.

For proposals and contested unresolved claims `observation` (bytes/instructions + half-open range), `interpretation` (only their proof), and `authority` (manifest/Splat/map/owner). Recheck payload bounds, opcode widths, element counts, exclusive function ends, live/cached/analyzer/distinctions, all name terms. Review reproduces evidence without trusting analyzer labels/prose.

A rename never changes width, signedness, pointer depth, volatility, ABI, storage, extent, packing, code/CFG shape, matching aids, flags, or addresses.

## Authority ceiling

- `symbol`/`type`: target only—sorted `symbols.txt` (one spelling/address), target `internal.h`/`symbols.c`, same-target references. No aliases or generated `symbols/psyq.c` edits.
- Shared fixed-RAM: only if already in `config/targets/shared/symbols.txt`, or recursive proof gives identical address/content class/runtime role in **every** consumer composing that map, atomically promote one spelling across declarations/bindings/annotations/references; inventory every composing Splat. Otherwise local; address/ref count is insufficient. This globalizes data contract, not function/source ownership.
- `docs`: named existing files under `docs/`, `AGENTS.md`, `README.md` only. Delete/correct dated lift counts, transient rankings, `out/` snapshots, dead links, duplicates; never relocate; preserve history/changelog. Durable facts → `docs/specs/`; policy → `docs/agents/`; scoped work → `docs/plans/`.
- `audit`: report `path`, contract, evidence, smallest repair, validation, human approval. Recursively discover descendant manifests and owned map/Splat/sources/support/headers/annotations; enumerate owned/named `*.h` and local include edges. Record target/header counts and paths/identities. Audit manifest-less shared config separately. Large `internal.h`, raw compiled spellings, semantic metadata-owned filenames are not drift.
- `relocate[-batch]`: move to `src/bof3/<class>/`, update manifest claims + Splat `@source`; any failure reverts all.

## Identity—never violate

- Every lift has parsable function-level `@behavior` and address-authoritative `@source`; no filename fallback. File/entry renames require evidence-gated transactions; never rename a Splat boundary address.
- Preserve `@behavior`/`@source`/`@kind` and evidence comments; correct stale tags in place. Use `/* @source 0x... @kind ... */`; `//` breaks gcc-2.6.3 objects.
- Never move target config or alter `source_dir`, load addresses, boundaries, SDK maps, public headers, `src/shared/`, `out/`, `build/`, `toolchains/`. Relocation atomically updates source/support/header/`psyq_source`, C-boundary `@source`, flag keys.
- Other organization requires plan + approval; else blocker, no edits.

## Transaction

1. Refuse overlap with a modified candidate unless the parent named that edit.
2. Naming: record spelling, same address/layout, binding, local references, evidence; atomically update map/declaration/binding/same-target references. Map + `WEAK_SYMBOL_AT` own addresses. A retained partial preserves body/status metadata verbatim and reports unchanged live non-baseline. Data declares `@kind: bss|rodata|string|table`. Shared exception updates all consumers, removes compatibility/self aliases, live-validates representative consumers per authored target family.
3. Docs: find the owning fact; change only its stale claim.
4. Parent phases: audit preflight → safe `bin/naming-audit prepare TARGET --repair` → audit/evidence graph → isolated symbol transaction → validation. Audit is read-only; children never switch mode. Classify mechanical repair / scoped plan / ownership-evidence blocker.
5. Verify no owned old spelling, unrelated target change, or broken edited link.
6. After retained map/Splat/reviewed/manifest transaction and normal symbol/Splat/build/byte gates, run both status commands. Either stale → one batch `bin/index --recover`; require both fresh; never rebuild per file.

## Post-loop cleanup

0. Run `bin/analysis-readiness [TARGET]` for the generated naming/type/macro readiness summaries and required-work graphs; close or explicitly retain its blockers before an application transaction. Refresh the disposable index only after authoritative edits pass, then rerun readiness.
1. `bin/symbols normalize TARGET --write`; `bin/symbols check TARGET`; sort `name = 0xADDRESS;`.
2. One evidence-gated rename at a time. Raw names need two corroborators; before a ceiling run focused PSX Rizin and recover stale evidence. Regeneration failure blocks.
3. Lifts: `src/bof3/<class>/`; manifest + `@source`/map/Splat own them, never ancestry. Prefer `relocate-batch`; never mid-match.
4. Each touched selector: fresh `bin/asm-diff TARGET@0xADDRESS --detail normal`, `bin/byte-match TARGET@0xADDRESS`; map/Splat change → `bin/splat TARGET`; fresh `bof3-review`. Failure → revert, never fix forward.

## Pipeline changes

Changes to `config/compiler/variants.json`, `config/compiler/object-flags.cmake`, `BOF3_OBJCOMPILER_`, `bin/cc`, maspsx, `bin/as`, or linker toolchain require:

    python -m pytest tools/python/tests/test_bin_cc_pipeline.py -v
    python -m pytest tools/python/tests/test_asm_link.py -v

Then live asm/byte checks for every affected lift. Source-only lifts exempt.
