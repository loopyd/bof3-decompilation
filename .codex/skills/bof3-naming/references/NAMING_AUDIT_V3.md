# Naming audit v3

## Run contract

`audit-target TARGET` owns one canonical target report; read-only except
caller-authorized disposable evidence/report writes. Never edit identities,
annotations, docs, source locations or lifts on this mode.

Run `bin/naming-evidence-run TARGET REPORT` with both tokens copied exactly from
the frozen cleanup request. Never construct, search for, substitute or accept a
caller-supplied report path. Missing/noncanonical state blocks with the explicit
initialization prerequisite `bin/naming-audit init-all out/reviews/plan-audit-naming`;
do not initialize implicitly or choose another report. Convert initializer rows
only to receipt-backed `exhausted` or `proposed` conclusions. Return proposals to
the parent for separately approved identity application.

Each invocation processes at most 10 rows within a 600-second shard wall and
120-second relative per-operation cap (`--deadline`). For a bounded parent run,
pass its original absolute monotonic `--work-deadline`; retain the separate cleanup
hard-stop. Follow [owner work deadlines](../../../../docs/specs/HARNESS.md#owner-work-deadlines):
expiry stops forward writes/finalization, not client cleanup; partial evidence
remains for inspection. Resume only within the original budget, with the same
canonical command and no replay of completed checkpoint rows. Keep output at most 8 KiB; cite report
and receipt paths with SHA-256, not report bodies. A shard/query success is not
target completion: require successful `bin/naming-audit validate TARGET REPORT`
without a transaction filter and `complete:true`. If collection/import used an
explicit evidence root, validation must use the same canonical absolute
`--evidence-root PATH`; never substitute roots or rewrite receipts. Incomplete
or blocked rows remain gaps even when the runner exits successfully.

For missing inventory rows in an existing canonical report, preview
`bin/naming-audit reconcile TARGET`, retaining the same explicit evidence root.
Only with reconciliation authority use `--apply`: append missing blocked rows
after full validation, preserving existing rows and other reports. Duplicate or
extra rows and invalid evidence block. This neither closes the audit nor permits
identity edits. Never use `init-all` for reconciliation: it replaces all reports.

Structural validation/capability exhaustion is not independent semantic acceptance.
Autonomous no-op/exhaustion requires the read-only parent-pinned
[terminal verification](../../../../docs/usage.md#naming-terminal-acceptance-read-only):
distinct preparation/reviewer/parent runs, exact current binding, retained ceiling
evidence and no unresolved executable leads. It neither rewrites history nor
authorizes identities. Selected-row acceptance never replaces the full-report gate.
Links to transaction/cosmetic modes are handoffs, not edit permission.

## Evidence gate

Retain names only with target-local address/layout and two independent corroborators: two consistent local accesses/calls; one + Rizin annotation; or proven layout/dispatch table + consistent use. Decompiler names, duplicate hashes, strings, comments, single sites, and `data-scan` counts are leads. Keep `D_XXXXXXXX`/`unk_XX`/`field_XX` if uncertain. Name data by proven content class: strings, pointer/handler table, or struct layout + consumers.

Under two corroborators is not a ceiling:

1. Before semantics, run `bin/naming-audit prepare TARGET`. `safe_metadata_repair` requires `bin/naming-audit prepare TARGET --repair`; its live `asm-diff`/`byte-match` must prove exactness before progress metadata is canonicalized, while ownership/layout stays blocked. Run row inventory with `bin/rev-query --json inventory TARGET`; emit `bof3.naming-audit/v3`. Validate with `bin/naming-audit validate TARGET REPORT.json --transaction KIND:NAME`; other blocks do not. Then run `bin/naming-audit verify TARGET REPORT.json --transaction KIND:NAME` to prove scope, old-name absence, and storage. `rev-query describe` owns payload/file/storage; `rev-query transaction-scope TARGET SYMBOL` owns transaction files.
2. Run `bin/rz-project status TARGET --json` and `bin/rev-query --json status`. If either is stale/unavailable, `bin/index --recover` reanalyzes **every** stale manifest snapshot, atomically rebuilds disposable `out/index/`, then recheck both—never one target only. Recovery touches only `out/reverse/`/`out/index/`. Failure blocks; name target, command, and smallest repair.
3. PS-X repair: original header outranks tools. Verify magic, `t_addr == manifest.load_address`, `t_size == payload size`, file offset `0x800`, and `runtime - load == payload offset`. Never analyze headers as payload or force mismatched maps. Malformed/truncated/mismatched images block the row; repair manifest/input/extraction, then recover/recheck. Raw overlays use offset `0`, never this formula.
4. When fresh, query `rev-query` calls/xrefs/symbols first. `rev-query xrefs TARGET@ADDRESS` supplies indexed data source/function/access kind/opcode. For reproducible bounded observations use `bin/rz-project query TARGET -c 'COMMAND'`; `open` is interactive only. Prove binding eligibility before semantics.
5. A function outside payload/boundaries is an import lead, not owned. Run `bin/rev-query --json owners TARGET@0xADDRESS`, inspect every plausible candidate, and prove owner from manifest load/range, original bytes, boundary, and runtime composition. Query calls/xrefs/variables/metrics; inspect the proven body with `bin/rz-project open OWNER`. `reviewed_range`/`analyzer_range`/`mapped_entry` provenance/confidence/containment are leads. Propose only in its owner contract; SDK exceptions remain explicit. Same addresses across unrelated overlays are separate; a resident executable may own a shared service.
6. Outside data ownership uses maps/Splat/load ranges plus original-byte access/initializer/consumer proof, not function `owners`. The sole data exception, shared fixed-RAM eligibility, is defined by [Identity transactions](IDENTITY_TRANSACTIONS.md#authority-ceiling).
7. Empty/incomplete xrefs or owners require bounded analysis per proven image: candidate/delay slots, `jal`, aligned pointer-table entries, neighboring handlers/state accesses, and original table bytes. A machine-code caller, callee body/state effects, dispatch layout, or state-machine consumer corroborates only with proven address model/boundaries. Analyzer/decompiler output is hypothesis; audit never mutates annotations.
8. Before `no-change`, go one semantic level beyond: table consumer/selector/neighbors; caller guards/transitions/result/stable arguments; transform/copy helper callers and source/destination roles; presentation helper callee + caller context; raw data's other consumer or initializer/use pair. Repeated unexplained shapes count once. Name the missing static fact. Runtime is optional when original-byte/layout/caller/consumer evidence suffices.
9. A semantic partial is eligible only for the spelling transaction in [Byte-safe cosmetics](BYTE_SAFE_COSMETICS.md#spelling-transaction-rung). Preserve body, ABI, address, boundary, compiler settings, `@status partial`, `@match`, and `@residual`; exactness is unnecessary, and matching edits cannot be bundled.

## Recursive inventory and audit authority

Recursively discover every descendant target manifest and its owned map, Splat, claimed sources/support files, headers, reviewed annotations, and source-local include edges. Enumerate all owned or named `*.h` files, including headers outside a single source include walk. Audit manifest-less shared configuration separately; never assign it by directory proximity.

Each finding reports these six audit-target fields exactly once: `path`, `contract`, `evidence`, `smallest repair`, `validation`, `human approval`. Separately record these recursive inventory counts and identities exactly once: target count, header count, target paths, header paths, resolved target identities, resolved header identities—the paths and identities deriving those counts. Exclude known false-positive drift: a large `internal.h`, raw compiled address spellings, and metadata-owned semantic filenames are not drift alone. Directory ancestry and filenames never grant source authority; explicit manifest claims, maps, Splat, and parsable function-level `@source`/`@behavior` metadata do.

## Audit evidence

Every raw inventory row appears once, proposed or unresolved. Each retained/proposed rename has `rename_evidence`:

- identity: selector, function/data kind, old/new, unchanged address/range; binding/source locations exactly equal `rev-query transaction-scope`—omit no caller/manifest; invent no binding;
- function: Rizin commands + callsite/instructions/delay slot/arguments/guards/result/transition; data: access/initializer/consumer commands + instructions/layout;
- runtime owner: outside function `owners` + manifest/load-range, bytes, boundary, composition; outside data maps/Splat/load ranges + byte/use proof. Local may use `N/A — image owns range` only with range proof;
- owner function: commands + callee instructions/delay slots/globals/tables/callees/consumers; owner data: accesses, initializer/consumers, class/layout;
- ID-typed observations; corroborators map IDs to distinct mechanisms (duplicate range/shape/mechanism counts once); `name_terms` maps every semantic word to IDs, else narrow/reject;
- partial: live status/match/residual + original-byte Rizin verification; C alone never counts;
- rejection: missing static fact + next bounded Rizin/original-byte command.

Every command executes and records passed/failed status, a repo-relative `out/reviews/evidence/` receipt, and verified SHA-256; never fabricate. Also record observed instructions/bytes/addresses/effects: commands alone are not evidence.

| Kind/locality | Required typed `rungs` |
|---|---|
| local function | `selected_range`, `selected_call`, `one_level_beyond` |
| imported function | `selected_call`, `owner_resolution`, `owner_body`, `one_level_beyond` |
| local data | `selected_range`, `selected_access`, `storage_class`, `one_level_beyond` |
| outside data | `selected_access`, `owner_resolution`, `owner_data`, `storage_class`, `one_level_beyond` |

| Rung | Proof |
|---|---|
| `selected_range` | selected half-open owned range and original bytes |
| `selected_call` | instruction-level callsite, including delay slot, arguments, guards, result, or transition as applicable |
| `selected_access` | instruction-level data read/write and effective address |
| `owner_resolution` | owning image/data contract from manifest, ranges, bytes, boundaries, and composition |
| `owner_body` | owner function's instructions, delay slots, globals/tables, callees, and effects |
| `owner_data` | accesses, initializer/consumers, class, and layout |
| `storage_class` | exact storage returned by `rev-query describe` |
| `one_level_beyond` | next selector/consumer/caller context required by evidence-gate step 8 |
| `partial_baseline` | live status, percentage, byte sizes, first mismatch, residual, and original-byte verification before a retained-partial spelling transaction |

Partial evidence requires `partial_baseline`. Tool-generated `required_work` covers indexed callers/callees/accesses/owners; for each item, record commands + typed observations and evidence-deduplicate it, or leave it open. Concrete discoveries expand this bounded graph. `exhausted` requires all closed; open/failed work means `blocked`, never optional ceiling work. `optional_work`/`ceiling_next_command` hold only post-ceiling experiments. Proposals record `semantic_status`, `transaction_status: ready|repairable|blocked`, and `readiness_blockers`; proposed data storage exactly equals `describe`.

For proposals and contested unresolved claims, separate `observation` (bytes/instructions + half-open range), `interpretation` (only their proof), and `authority` (manifest/Splat/map/owner). Recheck payload bounds, opcode widths, element counts, exclusive function ends, live/cached/analyzer distinctions, and all name terms. Review reproduces evidence without trusting analyzer labels/prose.

Immutable rename dimensions, their application, and shared fixed-RAM promotion are owned only by [Identity transactions](IDENTITY_TRANSACTIONS.md#authority-ceiling); this audit emits evidence and readiness, not edits.
