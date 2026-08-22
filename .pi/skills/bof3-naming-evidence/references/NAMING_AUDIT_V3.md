# Naming audit v3

## Evidence gate

Retain names only with target-local address/layout and two independent corroborators: two consistent local accesses/calls; one + Rizin annotation; or proven layout/dispatch table + consistent use. Decompiler names, duplicate hashes, strings, comments, single sites, `data-scan` counts are leads. Keep `D_XXXXXXXX`/`unk_XX`/`field_XX` if uncertain. Name data by proven content class: strings, pointer/handler table, or struct layout + consumers.

Under two corroborators is not a ceiling:

1. Before semantics run `bin/naming-audit prepare TARGET`. A `safe_metadata_repair` requires a `bin/naming-audit prepare TARGET --repair` run, whose live `asm-diff`/`byte-match` proves exactness before canonicalizing progress metadata; ownership/layout stays blocked. Then run row inventory: `bin/rev-query --json inventory TARGET`; emit `bof3.naming-audit/v3`. Validate via `bin/naming-audit validate TARGET REPORT.json --transaction KIND:NAME`; other blocks do not. Afterward run `bin/naming-audit verify TARGET REPORT.json --transaction KIND:NAME` to prove scope, old-name absence, storage. `rev-query describe` owns payload/file/storage; `rev-query transaction-scope TARGET SYMBOL` owns transaction files.
2. Run `bin/rz-project status TARGET --json` and `bin/rev-query --json status`. If either is stale/unavailable, `bin/index --recover` reanalyzes **every** stale manifest snapshot then atomically rebuilds disposable `out/index/`; recheck both, never one target only. Recovery touches only `out/reverse/`/`out/index/`. Failure blocks; name target/command/smallest repair.
3. PS-X repair: original header outranks tools. Verify magic, `t_addr == manifest.load_address`, `t_size == payload size`, file offset `0x800`, `runtime - load == payload offset`; never analyze headers as payload or force mismatched maps. Malformed/truncated/mismatched images block the row; repair manifest/input/extraction, then recover/recheck. Raw overlays use offset `0`, never this formula.
4. When fresh, query `rev-query` calls/xrefs/symbols first; `rev-query xrefs TARGET@ADDRESS` supplies indexed data source/function/access kind/opcode. Reproducible bounded observations use `bin/rz-project query TARGET -c 'COMMAND'`; `open` is interactive only. Prove binding eligibility before semantics.
5. A function outside payload/boundaries is an import lead, not owned: run `bin/rev-query --json owners TARGET@0xADDRESS`, inspect every plausible candidate, then prove owner via manifest load/range, original bytes, boundary, runtime composition. Query calls/xrefs/variables/metrics; inspect proven body with `bin/rz-project open OWNER`. `reviewed_range`/`analyzer_range`/`mapped_entry` provenance/confidence/containment are leads. Propose only in its owner contract; SDK exceptions remain explicit. Same addresses across unrelated overlays are separate; a resident executable may own a shared service.
6. Outside data ownership uses maps/Splat/load ranges plus original-byte access/initializer/consumer proof, not function `owners`. Shared fixed-RAM eligibility is the sole data exception and is defined directly by [Identity transactions](../../bof3-identity-maintenance/references/IDENTITY_TRANSACTIONS.md#authority-ceiling).
7. Empty/incomplete xrefs or owners require bounded analysis per proven image: candidate/delay slots, `jal`, aligned pointer-table entries, neighboring handlers/state accesses, original table bytes. A machine-code caller, callee body/state effects, dispatch layout, or state-machine consumer corroborates only with proven address model/boundaries. Analyzer/decompiler output is hypothesis; audit never mutates annotations.
8. Before `no-change`, go one semantic level beyond: table consumer/selector/neighbors; caller guards/transitions/result/stable arguments; transform/copy helper callers and source/destination roles; presentation helper callee + caller context; raw data's other consumer or initializer/use pair. Repeated unexplained shapes count once. Name the missing static fact. Runtime is optional when original-byte/layout/caller/consumer evidence suffices.
9. A semantic partial is eligible only for the spelling transaction explicitly defined by [Byte-safe cosmetics](../../bof3-identity-maintenance/references/BYTE_SAFE_COSMETICS.md#spelling-transaction-rung). Preserve body, ABI, address, boundary, compiler settings, `@status partial`, `@match`, and `@residual`; exactness is unnecessary and matching edits cannot be bundled.

## Recursive inventory and audit authority

Recursively discover every descendant target manifest and its owned map, Splat, claimed sources/support files, headers, reviewed annotations, and source-local include edges. Enumerate all owned or named `*.h` files, including headers not reached from a single source include walk. Audit manifest-less shared configuration separately rather than assigning it to a target by directory proximity.

Every finding reports the six audit-target fields exactly once: `path`, `contract`, `evidence`, `smallest repair`, `validation`, and `human approval`. Separately, the report records the recursive inventory counts and identities exactly once: target count, header count, target paths, header paths, resolved target identities, and resolved header identities — the paths and identities used to derive those counts. Exclude known false positives from drift: a large `internal.h`, raw compiled address spellings, and metadata-owned semantic filenames are not drift by themselves. Directory ancestry and filenames never supply source authority; explicit manifest claims, maps, Splat, and parsable function-level `@source`/`@behavior` metadata do.

## Audit evidence

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

Rung definitions are explicit: `selected_range` proves the selected half-open owned range and original bytes; `selected_call` proves an instruction-level callsite including delay slot, arguments, guards, result, or transition as applicable; `selected_access` proves an instruction-level data read/write and effective address; `owner_resolution` proves the owning image/data contract from manifest, ranges, bytes, boundaries, and composition; `owner_body` proves the owner function's instructions, delay slots, globals/tables, callees, and effects; `owner_data` proves accesses, initializer/consumers, class, and layout; `storage_class` proves the exact storage returned by `rev-query describe`; `one_level_beyond` proves the next selector/consumer/caller context required by evidence-gate step 8; `partial_baseline` records live status, percentage, byte sizes, first mismatch, residual, and original-byte verification before a retained-partial spelling transaction.

Partial evidence requires `partial_baseline`. Tool-generated `required_work` covers indexed callers/callees/accesses/owners; each item with commands + typed observations, evidence-deduplicate it, or leave it open. Concrete discoveries expand this bounded graph. `exhausted` requires all closed; open/failed work means `blocked`, never optional ceiling work. `optional_work`/`ceiling_next_command` hold post-ceiling experiments only. Proposals record `semantic_status`, `transaction_status: ready|repairable|blocked`, `readiness_blockers`; proposed data storage exactly equals `describe`.

For proposals and contested unresolved claims `observation` (bytes/instructions + half-open range), `interpretation` (only their proof), and `authority` (manifest/Splat/map/owner). Recheck payload bounds, opcode widths, element counts, exclusive function ends, live/cached/analyzer/distinctions, all name terms. Review reproduces evidence without trusting analyzer labels/prose.

Immutable rename dimensions, their application, and shared fixed-RAM promotion are owned only by [Identity transactions](../../bof3-identity-maintenance/references/IDENTITY_TRANSACTIONS.md#authority-ceiling); this audit emits evidence and readiness, not edits.
