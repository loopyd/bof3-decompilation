# Naming audit v3

Read the selected branch: [run](#run-contract), [gate](#evidence-gate),
[inventory](#recursive-inventory-and-audit-authority), [schema](#audit-evidence).

## Run contract

For `audit-target TARGET`, use one canonical report and write only authorized
disposable evidence/reports. Do not edit identities, annotations, docs, relocations or lifts.

Copy both tokens from the frozen request into `bin/harness naming evidence TARGET REPORT`.
Never construct, search, substitute or accept a caller-supplied path. Missing or
noncanonical state blocks on `bin/harness naming init-all out/reviews/plan-audit-naming`;
never initialize implicitly or choose another report. Convert initializer rows only
to receipt-backed `exhausted`/`proposed` conclusions. Parent approval is required for
identity application.

Process at most 10 rows and emit at most 8 KiB per invocation. Cite report/receipt
paths and SHA-256, not bodies. Resume the canonical command with retained history;
do not replay completed checkpoint rows. Require successful unfiltered
`bin/harness naming validate TARGET REPORT` and `complete:true` for target closure.
Validate explicit collection/import roots with the same canonical absolute
`--evidence-root PATH`; preserve roots/receipts. Runner or shard/query success
cannot close incomplete/blocked rows.

Preview missing inventory rows with `bin/harness naming reconcile TARGET` and the
same explicit evidence root. Only authorized `--apply` appends blocked rows after
full validation, preserving existing rows and other reports. Duplicate/extra rows
or invalid evidence block. Reconciliation neither closes the audit nor permits
identity edits. Never reconcile with `init-all`: it replaces all reports.

For retained inventory drift, require a separately reviewed recovery successor.
Preview `bin/harness naming reconcile TARGET --successor` with externally pinned
`--expected-report-sha256` and `--expected-summary-sha256` and the retained evidence root.
Preserve predecessor/receipts and valid rows. Refresh only fully unreviewed
initializer gaps; record obsolete raw identities with current named bindings.
Closed work, observed evidence and invalid conclusions block refresh.
The preview's `plan` contains the complete recovery record, including replacement
and added rows. Review it against `plan_sha256` before publication.

After independent review, repeat with `--apply` and retained `--expected-plan-sha256`.
Let the owner publish an immutable generation/distinct inventory checkpoint and
activate accounting by compare-and-swap. Inspect ambiguous publication before retrying.
Recovery verifies no old applications or semantics. Verify the full successor and
no-change reconciliation preview before further naming.

Exact-capability validation recognizes exact historical optional owner command hints.
Require receipt/provenance/journal/recomputed-capability checks. Reject mixed hints
or other row changes; never normalize stored receipts/rows in place.

Structural validation/capability exhaustion grants no semantic acceptance.
Autonomous no-op/exhaustion requires read-only parent-pinned
[terminal verification](terminal-acceptance.md):
distinct preparation/reviewer/parent runs, exact current binding, retained ceiling
evidence and no unresolved executable leads. It neither rewrites history nor authorizes
identities. Selected-row acceptance never replaces the full-report gate.
Transaction/cosmetic links grant no edit permission.

## Evidence gate

Retain names only with target-local address/layout and two independent corroborators: two consistent local accesses/calls; one + Rizin annotation; or proven layout/dispatch table + consistent use. Decompiler names, duplicate hashes, strings, comments, single sites, and `data-scan` counts are leads. Keep `D_XXXXXXXX`/`unk_XX`/`field_XX` if uncertain. Name data by proven content class: strings, pointer/handler table, or struct layout + consumers.

Under two corroborators is not a ceiling:

1. Before semantics run `bin/harness naming prepare TARGET`; `safe_metadata_repair` also needs `--repair`, and its live `asm-diff`/`byte-match` must prove exactness before progress metadata is canonicalized, while ownership/layout stays blocked. Row inventory: `bin/harness analysis query --json inventory TARGET`; emit `bof3.naming-audit/v3`. Validate with `bin/harness naming validate TARGET REPORT.json --transaction KIND:NAME`; other blocks do not. `bin/harness naming verify TARGET REPORT.json --transaction KIND:NAME` then proves scope, old-name absence and storage. `bin/harness analysis query` owns payload/file/storage via `describe` and transaction files via `transaction-scope TARGET SYMBOL`.
2. Run `bin/harness analysis rz-project status TARGET --json` and `bin/harness analysis query --json status`. If either is stale/unavailable, `bin/harness analysis index --recover` reanalyzes **every** stale manifest snapshot, atomically rebuilds disposable `out/index/`, then recheck both—never one target only. Recovery touches only `out/reverse/`/`out/index/`. Failure blocks; name target, command, and smallest repair.
3. PS-X repair: original header outranks tools. Verify magic, `t_addr == manifest.load_address`, `t_size == payload size`, file offset `0x800`, and `runtime - load == payload offset`. Never analyze headers as payload or force mismatched maps. Malformed/truncated/mismatched images block the row; repair manifest/input/extraction, then recover/recheck. Raw overlays use offset `0`, never this formula.
4. When fresh, query `bin/harness analysis query` calls/xrefs/symbols first. `bin/harness analysis query xrefs TARGET@ADDRESS` supplies indexed data source/function/access kind/opcode. For reproducible bounded observations use `bin/harness analysis rz-project query TARGET -c 'COMMAND'`; `open` is interactive only. Prove binding eligibility before semantics.
5. A function outside payload/boundaries is an import lead, not owned. Run `bin/harness analysis query --json owners TARGET@0xADDRESS`, inspect every plausible candidate, and prove owner from manifest load/range, original bytes, boundary, and runtime composition. Query calls/xrefs/variables/metrics; inspect the proven body with `bin/harness analysis rz-project open OWNER`. `reviewed_range`/`analyzer_range`/`mapped_entry` provenance/confidence/containment are leads. Propose only in its owner contract; SDK exceptions remain explicit. Same addresses across unrelated overlays are separate; a resident executable may own a shared service.
6. Outside data ownership uses maps/Splat/load ranges plus original-byte access/initializer/consumer proof, not function `owners`. The sole data exception, shared fixed-RAM eligibility, is defined by [Identity transactions](identity-transactions.md#authority-ceiling).
7. Empty/incomplete xrefs or owners require bounded analysis per proven image: candidate/delay slots, `jal`, aligned pointer-table entries, neighboring handlers/state accesses, and original table bytes. A machine-code caller, callee body/state effects, dispatch layout, or state-machine consumer corroborates only with proven address model/boundaries. Analyzer/decompiler output is hypothesis; audit never mutates annotations.
8. Before `no-change`, go one semantic level beyond: table consumer/selector/neighbors; caller guards/transitions/result/stable arguments; transform/copy helper callers and source/destination roles; presentation helper callee + caller context; raw data's other consumer or initializer/use pair. Repeated unexplained shapes count once. Name the missing static fact. Runtime is optional when original-byte/layout/caller/consumer evidence suffices.
9. A semantic partial is eligible only for the spelling transaction in [Byte-safe cosmetics](byte-safe-cosmetics.md#spelling-transaction-rung). Preserve body, ABI, address, boundary, compiler settings, `@status partial`, `@match`, and `@residual`; exactness is unnecessary, and matching edits cannot be bundled.

## Recursive inventory and audit authority

Inventory every descendant manifest and owned map, Splat, claimed sources/support
files, headers, reviewed annotations and local include edges. Include all owned/named
`*.h`, including outside the source include walk. Audit manifest-less shared configuration
separately; directory proximity grants no ownership.

Each finding reports `path`, `contract`, `evidence`, `smallest repair`, `validation` and `human approval` exactly once. Record target count, header count, target paths, header paths, resolved target identities and resolved header identities exactly once; counts derive from those paths/identities. A large `internal.h`, raw compiled address spellings or metadata-owned semantic filenames alone are not drift. Directory ancestry and filenames grant no source authority; require explicit manifest claims, maps, Splat and parsable function-level `@source`/`@behavior` metadata.

## Audit evidence

Account every raw row exactly once: proposed or unresolved. Each retained/proposed
rename requires `rename_evidence`:

- identity: selector, function/data kind, old/new, unchanged address/range; binding/source locations exactly equal `bin/harness analysis query transaction-scope`—omit no caller/manifest; invent no binding;
- function: Rizin commands + callsite/instructions/delay slot/arguments/guards/result/transition; data: access/initializer/consumer commands + instructions/layout;
- runtime owner: outside function `owners` + manifest/load-range, bytes, boundary, composition; outside data maps/Splat/load ranges + byte/use proof. Local may use `N/A — image owns range` only with range proof;
- owner function: commands + callee instructions/delay slots/globals/tables/callees/consumers; owner data: accesses, initializer/consumers, class/layout;
- ID-typed observations; corroborators map IDs to distinct mechanisms (duplicate range/shape/mechanism counts once); `name_terms` maps every semantic word to IDs, else narrow/reject;
- partial: live status/match/residual + original-byte Rizin verification; C alone never counts;
- rejection: missing static fact + next bounded Rizin/original-byte command.

Execute every cited command; record passed/failed status, repo-relative
`out/reviews/evidence/` receipt, verified SHA-256 and observed instructions/bytes/
addresses/effects. No fabricated receipts; command list alone is not evidence.

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
| `storage_class` | exact storage returned by `bin/harness analysis query describe` |
| `one_level_beyond` | next selector/consumer/caller context required by step 8 |
| `partial_baseline` | live status, percentage, byte sizes, first mismatch, residual, and original-byte verification before a retained-partial spelling transaction |

Instruction capture supports canonical call wrappers and 8..32-instruction acyclic
BEQ/BNE callers with forward local jumps. Require complete path/slot coverage,
settled constant-address halfword reads, known selected-call arguments and a matching
saved-RA frame on every path. The selected call must precede opaque calls on its
path. Later calls leave outgoing v0/v1 unresolved. Return transitions explicitly
assume normal memory/call completion and preservation of SP and the saved-entry-RA
cell. Unsupported instructions, load timing, loops, indirect flow and post-call
dependencies are named `selected_call` gaps. Replay recomputes facts from original
bytes. These facts neither close other rungs nor establish a C return contract;
function conclusion admission remains disabled. `owner_body` retains its separate
wrapper and global-pointer zero-byte-store shapes.

Partial evidence requires `partial_baseline`. Tool-generated `required_work` covers indexed callers/callees/accesses/owners. Record each item's commands and typed observations with evidence deduplication, or leave it open. Concrete discoveries expand the bounded graph. `exhausted` requires all closed; open/failed work is `blocked`, never optional. `optional_work`/`ceiling_next_command` hold only post-ceiling experiments. Proposals retain `semantic_status`, `transaction_status: ready|repairable|blocked` and `readiness_blockers`; proposed data storage equals `describe` exactly.

Proposals and contested unresolved claims separate `observation` (bytes/instructions, half-open range), `interpretation` (their proof only) and `authority` (manifest/Splat/map/owner). Recheck payload bounds, opcode widths, element counts, exclusive function ends, live/cached/analyzer distinctions and all name terms. Review reproduces evidence without trusting analyzer labels/prose.

Only [Identity transactions](identity-transactions.md#authority-ceiling) owns immutable rename dimensions, application and shared fixed-RAM promotion. This audit emits evidence/readiness, not edits.
