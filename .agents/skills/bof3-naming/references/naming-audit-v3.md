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
to receipt-backed `exhausted`/`proposed` conclusions. Omit `initializer_state` and
`conclusion_provenance` from authored terminal rows. `bin/harness naming conclude`
adds receipt-bound provenance during import. Parent approval is required for
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

Exact-capability validation requires current code-owned command hints plus receipt,
provenance, journal and recomputed-capability checks. Retired command spellings
block acceptance. Preserve stored receipts and rows unchanged for reviewed recovery.

Structural validation/capability exhaustion grants no semantic acceptance.
Autonomous no-op/exhaustion requires read-only parent-pinned
[terminal verification](terminal-acceptance.md):
distinct preparation/reviewer/parent runs, exact current binding, retained ceiling
evidence and no unresolved executable leads. It neither rewrites history nor authorizes
identities. Selected-row acceptance never replaces the full-report gate.
Transaction/cosmetic links grant no edit permission.

An affected caller source can contain several functions. Every record must be
exact, the source must have one explicit lift owner, and its records, manifest
claims, Splat boundaries and compiled symbols must agree. Retain each caller
identity even when the transaction scope lists their shared source path once.
Each caller needs a finite, four-byte-aligned reviewed function range. Native
reports must cover that complete range; truncated or oversized results reject.

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

Each analyzer command runs in a separate owned Rizin process with the current
target replay and stdin EOF. Capture drains both output pipes through exit and
confirmed descendant cleanup before starting the next command. There is no stdin
command stream or framing parser. Interrupted batches retain remaining queued
commands without replaying completed work.
Unconfirmed cleanup retains the process handle and blocks retry until close succeeds.

Instruction capture supports canonical call wrappers and 8..512-instruction acyclic
BEQ/BNE callers with forward local jumps. Complete caller ranges are limited to
2,048 bytes; path and expression bounds remain. Require complete path/slot
coverage, settled constant-address halfword reads, known selected-call arguments and a matching
saved-RA frame on every path. The selected call must precede opaque calls on its
path. Later calls leave outgoing v0/v1 unresolved. Return transitions explicitly
assume normal memory/call completion and preservation of SP and the saved-entry-RA
cell. Straight-line wrappers permit one matching allocation and saved-RA store
in order before the call. Pre-call SB/SH require constant addresses and values,
aligned main RAM and no alias with captured caller bytes or the selected callee
entry word. Retain ordered width-truncated stores, including repeated addresses.
Stack and other code/data nonaliasing remain explicit premises, not proved facts;
shared owner-body output preserves them. Delay-slot stores and unresolved store
values are refused. Constant wrapper arguments use folded literal facts; older
expression projections require recapture, not fallback. Unsupported instructions,
load timing, loops, indirect flow and post-call dependencies are named
`selected_call` gaps. Only generated caller work authorizes call facts. The selected body's complete validated capture supplies `selected_range`
with its boundary, byte count, original hash and image/layout binding.
Supported complete acyclic leaves also supply `body_effects`: forward BEQ/BNE/J,
ordinary slots, terminal JR RA, constant-address LH/LHU/SB/SH and supported integer
operations. Retain all words, guards, ordered reads and truncated stores. Sample
branches before slots; keep rereads distinct. Require settled entry loads, normal
mapped accesses and stable code and standard RAM mappings. Unsupported flow,
instructions, addresses, load timing or code-overlapping RAM-alias stores retain
`body_effects_gap`. These conditional effects prove no path feasibility, object
extent, gameplay meaning or C return contract. Range validity survives a body gap.
A two-way zero/nonzero halfword-state dispatch with alternative first callees and
an identical later call suffix supplies `one_level_beyond` caller context. It proves
conditional invocation and continuation, not state meaning or callee effects.
Journal and conclusion validation share indexed-record and live original-instruction
recomputation. Their caller-owned plan, receipt, namespace and freshness checks
remain required. Positive source coverage follows the row's kind, locality and
partial-baseline profile; coverage alone establishes no receipt independence.
One receipt cannot close two rungs; a dispatcher context needs a separate
caller receipt for `selected_call`. Range ownership proves no name term or C return
contract. Generic conclusion admission requires a complete positive profile, no
open or unavailable evidence, and exact current replay. Corroborators must bind
their observations to distinct receipt sources and mechanisms. Capability admission
grants no semantic approval. `owner_body` supports a canonical single-call wrapper
or a complete 16..65536-byte body of four-word LUI, LW, NOP, SB-zero groups. The final
group uses JR RA instead of NOP, with SB-zero in its return delay slot. Facts prove
direct effects only, without object extent or transitive callee effects.
Every missing profile rung, including `partial_baseline`, retains a capability
gap. Decode refusals remain open despite positive facts from other captures for
that rung. `owner_resolution` and `owner_data` have no positive typed producer.
Imported function and data rows remain blocked. Ordinary single-function partials
can obtain `partial_baseline` from the canonical native pair. It binds declared
progress separately from measured sizes, instruction counts, percentage and first
mismatch, with current original and linked function bytes. Only the assembly receipt
carries this fact. Journal and conclusion replay rerun both commands and compare
the observations. Unsupported partial shapes retain a named gap.
Overlapping candidate ranges and planned native checks cannot close those rungs.
Instruction capture must complete the generated plan. A capture failure aborts
the row without a committed receipt. Failed receipts cannot close work.
Only instruction captures receive instruction observations. Partial `asm-diff` and
`byte-match` receipts retain complete JSON stdout; a shared selector grants no
instruction facts to a native check.

Data collection requires a current typed receipt for every indexed operation.
`data_original_range` binds the whole containing reviewed data or rodata segment,
up to 64 KiB, to the current image, layout and replay identities. Starting at the
aligned selector, it records up to 16 consecutive original words that each name
one unambiguous reviewed function start, plus the stopping reason. This prefix
proves original content, not object extent, total count or live pointer contents.
Describe supplies `storage_class` on a separate receipt. Unsupported ranges,
including BSS, retain a named gap. Complete decoded consumer reads must fit within
the prefix; every unsupported required consumer remains a gap. Initializer and
consumer receipts use distinct mechanisms with shared image and layout premises.
Function and data conclusions use the same current-work, typed-rung and replay
gates. Missing, stale or superseded captures require recapture without fallback.

Stack-copy dispatch consumers require a complete original body. The decoder
accounts for delayed loads, saved return address, copies present at the indexed
load, global stores, the indirect call and its slot, and stack restoration.
Facts retain every memory access and explicit call-preservation assumptions.
Copied word spans prove neither object extent nor total table length; byte
selectors outside the copied slots remain unresolved. Unsupported instructions,
transfers or register flow leave a named gap. Replay requires the current full-body
projection; older projections require recapture.

Imported function discovery includes target-local unresolved `static_jal` index
edges as deduplicated caller work, preserving every owner candidate. The destination
address supplies a lookup key, not a callee identity or runtime-owner proof.
Other unresolved edge kinds require further discovery. Each caller still needs
its complete reviewed capture and supported decode; an indexed JAL alone closes
no semantic rung.

Partial evidence requires `partial_baseline`. Tool-generated `required_work` covers indexed callers/callees/accesses/owners. Record each item's commands and typed observations with evidence deduplication, or leave it open. Concrete discoveries expand the bounded graph. `exhausted` requires all closed; open/failed work is `blocked`, never optional. `optional_work`/`ceiling_next_command` hold only post-ceiling experiments. Proposals retain `semantic_status`, `transaction_status: ready|repairable|blocked` and `readiness_blockers`; proposed data storage equals `describe` exactly.

Proposals and contested unresolved claims separate `observation` (bytes/instructions, half-open range), `interpretation` (their proof only) and `authority` (manifest/Splat/map/owner). Recheck payload bounds, opcode widths, element counts, exclusive function ends, live/cached/analyzer distinctions and all name terms. Review reproduces evidence without trusting analyzer labels/prose.

Only [Identity transactions](identity-transactions.md#authority-ceiling) owns immutable rename dimensions, application and shared fixed-RAM promotion. This audit emits evidence/readiness, not edits.
