# Tool usage

Work on one `TARGET@0xADDRESS` at a time. Commands keep terminal output short;
full generated evidence is written under `out/`.

## Output budget

Context-heavy commands accept `--detail minimal|normal|full`:

| Level | Use | Output |
| --- | --- | --- |
| `minimal` | candidate scouting, small models | decision fields or one summary line |
| `normal` | daily iteration | labeled metrics or the first bounded diff hunk |
| `full` | debugging/tool development | complete rows, function records, or diff |

`normal` is the text default. Plain `--json` remains full for automation.
`-o FILE` always writes the complete artifact. For payload commands such as
`m2c`, use `-o` and open only the file you need to edit.

## Naming conclusion import

`bin/naming-audit conclude TARGET REPORT INPUT [--evidence-root ABSOLUTE_PATH]`
imports one expert-authored, receipt-bound conclusion. Omit `--evidence-root`
for the production-default evidence tree. When collection used an explicit
root, pass that same absolute path to import and replay; a relative, omitted,
wrong, or repeated explicit root fails closed. Supply exactly one canonical
absolute path: one leading slash, no `.`/`..`, duplicate separators, or trailing
separator; the filesystem root `/` is not a valid evidence root. No existing
path component may be a symlink. A
nonexistent suffix is created only beneath its nearest symlink-free ancestor.

Initialize the campaign report set with `bin/naming-audit init-all out/reviews/plan-audit-naming`. Then `bin/agent-context cleanup audit-target TARGET` emits the one canonical report path; audit agents pass that `TARGET` and `REPORT` unchanged to the lower-level runner. Generated `out/` state remains disposable, while the explicit report argument binds downstream provenance.

```sh
bin/agent-context cleanup audit-target TARGET
# Copy the emitted cleanup-request `report` value unchanged.
ROOT="$(mktemp -d)"
bin/naming-evidence-run TARGET REPORT --rows KIND:NAME --evidence-root "$ROOT"
bin/naming-audit conclude TARGET REPORT INPUT --evidence-root "$ROOT"
# Exact replay uses the same report, input, and explicit root.
bin/naming-audit conclude TARGET REPORT INPUT --evidence-root "$ROOT"
```

`INPUT` uses `bof3.naming-conclusion/v1`. The top-level `report_sha256` is the
current report CAS digest: import fails if the report bytes have changed. The
source `report_digest` is the initializer/report digest recorded by collection;
it remains stable as evidence provenance after a successful conclusion changes
the current report. `source` also binds the collected `bof3.naming-evidence/v2`
payload and the code-owned terminal capability. Its `conclusion` is a
shape-only row illustrated by the checked fixtures
[`naming-conclusion-exhausted.json`](../tools/python/tests/fixtures/naming-conclusion-exhausted.json)
or [`naming-conclusion-proposed.json`](../tools/python/tests/fixtures/naming-conclusion-proposed.json).
These files validate row shape only and are not importable fixture sets. Conclusion import remains fail-closed unless the supported typed analyzer can
recompute every required rung from bound native output. Collection receipts,
checkpoints, and manifests provide integrity and crash-recovery bookkeeping,
but they are writable local artifacts and cannot authenticate semantic facts.
The initial data-access slice derives range, storage, and exact access width;
open or unavailable rungs still prevent `exhausted` and `proposed` imports.
The envelope stores the completed row under `conclusion`:

```json
{"schema":"bof3.naming-conclusion/v1","target":"TARGET","report_sha256":"CURRENT_REPORT_SHA256","source":{"report":"out/reviews/TARGET.json","report_digest":"INITIALIZER_REPORT_SHA256","evidence":"out/reviews/evidence/RUN/payload.json","evidence_sha256":"EVIDENCE_SHA256","conclusion_capability":"CAPABILITY_ID"},"conclusion":{}}
```

A proposed conclusion uses the same envelope and evidence bindings, sets
`rung_status` to `proposed`, and additionally supplies `new_name`, `identity`,
at least two independent corroborators, and `storage` for data. The schema is
retained for the future trusted analyzer seam; it does not bypass the current
fail-closed gate. Failed or unavailable work remains blocked. The complete
resulting report is validated; proposals also receive isolated transaction
validation. Replay is a no-op only when the exact stored conclusion and its
original evidence path/digest match, even though the report digest changed.
Missing/stale evidence, forged cross-target receipts, authored-row overwrite,
and concurrent report changes fail closed. Untouched rows retain structural
values and order, and the report file mode is preserved.

## Ordered workflow

### 1. Prepare the repository

```sh
just setup
just doctor
```

First-time setup requires the host tools listed in the
[README prerequisites](../README.md#prerequisites); see that list for which
`cmake`/`7z` roles are required and why. `setup` validates one complete CUE/BIN
set under `inputs/external/`. If it is not present, it accepts `inputs/external/BreathOfFireIIIv1.1.7z` and extracts it
to the private-assets cache. It then downloads/stages the required toolchains,
extracts reviewed target images, and validates the result. `doctor` repeats
setup validation. Run `bin/symbols check` separately to validate symbol maps.

Use `bin/bof3-disk` to inspect original disc media, `bin/emi-ex` to list or
extract an EMI archive, and `bin/str-media inspect|validate|convert` for STR
media. These are acquisition tools, not function-analysis inputs.

### 2. Bootstrap one new EMI target

```sh
bin/emi-target BIN/BATTLE/BATL_END.EMI#0
bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply
bin/symbols check
bin/splat TARGET
```

`emi-target` previews before `--apply`; it refuses an existing target and
creates a new identity plus bin-only reviewed layout. `symbols` validates
target-local maps. `splat` regenerates assembly and linker inputs for new or
existing reviewed targets; add `--verbose` only for complete Splat diagnostics.

### 3. Build analysis evidence

```sh
bin/rz-project analyze TARGET
bin/rz-project status TARGET
just index
```

`rz-project` keeps each independently loaded image isolated. Analyze every
stale or missing target snapshot before `just index`; indexing fails unless all
manifest snapshots are fresh, then atomically rebuilds the cross-target cache.
Use `bin/rz-project open TARGET` only for interactive investigation.

PsyQ SDK evidence and application:

`bin/harness` is the permanent, narrow PsyQ object-signature evidence adapter.
Its only command family is `psyq {scan|calls|proposal}`; symbol-map mutation
remains under `bin/symbols`.

```sh
# Gather provenance (signatures identify objects; official headers own declarations)
bin/psyq-import --example
bin/harness psyq scan --all
bin/harness psyq calls --all
bin/harness psyq proposal --all

# Apply reviewed provenance, then regenerate and audit bindings
bin/symbols import-psyq out/psyq/proposal.json --all-qualified --write
bin/symbols psyq-bindings --write          # regenerate manifest-owned psyq_source bindings
bin/symbols psyq-report TARGET             # which SDK symbols the code references
```

The SDK maps live in `config/sdk/psyq-{slus,logo}.txt`; a target selects its
space via the manifest `[psyq] space` key (default `slus`). `import-psyq` writes
reviewed exact candidates into the space's SDK map; `psyq-bindings` regenerates
the compiled manifest-owned `psyq_source` (e.g. `src/bof3/support/slus_psyq.c`)
from it. Nothing edits maps without `--write`.

### 3b. Target analysis (freshness → rebuild → query)

Run the existing commands explicitly, in order:

```sh
bin/rz-project status TARGET
bin/index
bin/rev-query quick-wins --target TARGET
# Or: bin/rev-query metrics TARGET@0xADDRESS --detail normal
```

Stop if `rz-project status` reports a stale snapshot; rebuild that target's
snapshot before running `bin/index`. Rebuild the index only after freshness
succeeds, then query the requested target without touching reviewed maps.
`rev-query` refuses stale snapshot/index evidence. Use `--exclusions`
to inspect rows rejected by canonical-code checks; use `--detail full` for
complete rows.

### 4. Select one function

```sh
bin/rev-query quick-wins --unlifted --detail minimal --limit 5
bin/rev-query leafs --unlifted --detail minimal --limit 5
bin/rev-query duplicates --unlifted --detail normal --limit 5
bin/rev-query metrics TARGET@0xADDRESS --detail normal
bin/rev-query quick-wins --exclusions --detail full --limit 0
```

Use `--exclusions` on any ranking command to inspect target-qualified rows
rejected by canonical code checks; it reports `candidate_exclusion` instead of
ranked candidates. Use `quick-wins` for low effort, `hotspots` for caller impact, `leafs` for
bounded call dependencies, `pareto` for visible effort/value trade-offs, and
`duplicates` for exact-byte leverage. Rankings are hints, not promotion proof.

Supporting queries:

```sh
bin/rev-query calls TARGET@0xADDRESS
bin/rev-query xrefs TARGET@0xADDRESS
bin/rev-query owners TARGET@0xADDRESS
bin/rev-query describe TARGET@0xADDRESS
bin/rev-query transaction-scope TARGET SYMBOL
bin/rev-query inventory TARGET
bin/rev-query symbols NAME
bin/rev-query variables NAME
bin/rev-query types [NAME] [--target TARGET] [--untyped] [--detail full]
bin/rev-query type-uses [NAME] [--target TARGET]
bin/rev-query type-candidates [--target TARGET] [--status blocked] [--kind KIND]
bin/rev-query status
```

`xrefs` matches exact target-qualified destinations, not possible indexed bases.
Data extraction recognizes direct LUI/%lo uses within 12 instructions; it does not
propagate materialized pointers or resolve dynamic indexing. An `address` row is
not a load/store, and empty results cannot prove no accesses or complete writer
coverage. The reviewed table-consumer decoder separately proves its fixed original
loads, not all consumers. Allowlisted structural `exhausted` capabilities are not
independent semantic ladder exhaustion or current no-op acceptance.

`owners` combines reviewed Splat ranges, analyzer ranges, and exact mapped-entry leads; provenance and confidence are hypotheses, never ownership authority. `xrefs` includes call references and decoded data accesses with source address, access kind, and opcode. `describe` reports canonical payload/file offsets, reviewed Splat boundary, exact symbol, and references. `types` inventories target-owned declarations plus the explicitly shared base scalar aliases; full detail includes fields, layout constraints, conflicts, diagnostics, and provenance. `type-uses` reports declaration/use relationships. `type-candidates` reports conservative representation and semantic leads only: every inferred aggregate, field, array, prototype, or class-like receiver/dispatch row remains blocked until its independent evidence gaps are closed.

Macro discovery, near-duplicate ranking, consumer inspection, and resolution commands
are documented only in [MACROS.md](specs/MACROS.md).

Type storage/layout inference requires a decoded load/store with a recognized
width and matching opcode/access kind. Address materialization (`addiu`/`ori`)
remains in reference queries, but cannot establish storage widths, layout fields,
regions or array strides. Aggregate/array `end` is the exclusive end of the
observed access span, including access widths—not a proven object/array extent.
Prototype and blocked semantic receiver/dispatch leads remain separate.
Reverse index v14 invalidates older cached inference; refresh explicitly with
`bin/index` after snapshot freshness checks. Queries never rebuild it implicitly.

`bin/analysis-readiness [TARGET]` is the bounded aggregate checkpoint. By default it reports snapshot/index freshness and stale facts, then summary work graphs and exact naming, type, and macro counts; `TARGET` restricts every inventory, debt, candidate, and work count to that target. Use `--detail full` only when the exhaustive candidate rows, blockers, fingerprints, and generated naming work are required. Both modes retain the `bof3.analysis-readiness/v2` schema and differ only in `work_graph` detail. The command is read-only and prints `bin/index --recover` when authoritative inputs have made the disposable index stale. Recovery is explicit so reviewed transactions pass before index refresh.

Naming audits start with readiness preflight, then use `bof3.naming-audit/v3` typed rungs, generated required work, typed corroborators, canonical transaction scope/storage, and digest-verified receipts. A mechanically safe exact-progress repair requires live proof:

```sh
bin/naming-audit prepare TARGET
bin/naming-audit prepare TARGET --repair
bin/naming-audit prepare TARGET --repair --rows KIND:NAME[,KIND:NAME...]
bin/naming-audit init TARGET out/reviews/audit.json
bin/naming-audit validate TARGET out/reviews/audit.json --transaction function:func_80100000
# Apply the isolated spelling transaction, then verify current truth:
bin/naming-audit verify TARGET out/reviews/audit.json --transaction function:func_80100000
bin/naming-audit validate TARGET out/reviews/audit.json
```

`init` writes every current raw inventory row once as an explicit blocked evidence gap, including tool-generated required work and the next bounded command for each open typed rung; auditors replace those rows only with receipt-backed exhausted or proposed conclusions. The isolated pre-transaction check ignores unrelated blocked rows but rejects incomplete locations, open mandatory work, malformed metadata, noncanonical storage, or overlapping proposals. `bin/naming-audit verify` derives scope by the recorded address and new spelling, proves every reported location migrated, the old spelling is absent, and data storage is unchanged.

### Frozen naming postapply lifecycle

`postapply-gates` executes only normalize/write, symbol check, Splat, target build,
asm-diff JSON/full and byte-match JSON, in that order (120 seconds and 2 MiB
output per command). It accepts no supplied success status and needs no review.
It retains execution outputs, exact report bytes/hash binding, derived input
closure, modes/absence, normalization transition, final Ninja graph and index
fingerprint. The closure includes transaction/provenance paths, all CMake source
and header/manifest glob inputs, target binary/Splat/maps/reviewed includes,
compiler configuration, installed configured variants, native tooling and
`out/catalog/emi.json` (including absence). Recovery that changes this catalog
invalidates prior gates/review; rerun rather than rebind receipts.
Unrelated closure drift also requires rerun. Configured compiler variants must
already be installed; this lifecycle does not authorize installs.
Object-flags CMake is restricted to blank lines, `#` line comments (no bracket
comment openers), and lowercase, single-line `set(BOF3_OBJFLAGS_KEY FLAGS...)` or
`set(BOF3_OBJCOMPILER_KEY ID)` assignments. Indentation, horizontal whitespace and
trailing line comments are allowed. Keys contain only ASCII letters, digits and
underscores; IDs start alphanumeric and contain only ASCII letters, digits,
periods, underscores and hyphens. Flags start with `-` and contain only ASCII
letters, digits and `_,=.+-`. Quoting, escapes, lists, variable expansion,
multiline/uppercase commands and all other CMake content reject before gates.
Only exact in-payload, target-local FUNCTION proposals with explicit source
claims, Ninja and no companion overlays or environment toolchain overrides are
supported. Other classes fail closed. No rename or index recovery is performed.

After an authorized cleaner freezes its rollback snapshot and applies **only**
the approved spelling, use the same canonical target/report/evidence root:

```sh
bin/naming-audit postapply-gates TARGET REPORT --transaction function:OLD --implementation-run-id IMPLEMENTATION_RUN --evidence-root "$ROOT"
# Retain printed gates.json. Prove finalized scope/old spelling/body preservation.
bin/rz-project status TARGET --json
bin/rev-query --json status
# Parent only: if stale, one sanctioned bin/index --recover, then repeat both.
bin/analysis-readiness TARGET
git diff --check
# Compare staged index with cleaner snapshot; independent native reviewer now
# inspects snapshot, current files, native gates and readiness, NOT final verify.
# Trusted supervising parent writes the explicit attestation below after review.
bin/naming-audit postapply-review TARGET REPORT --transaction function:OLD --gates GATES_JSON --parent-attestation PARENT_JSON --evidence-root "$ROOT"
bin/naming-audit verify TARGET REPORT --transaction function:OLD --post-apply-receipts REVIEWED_BUNDLE --evidence-root "$ROOT"
```

Parent JSON is a closed `bof3.naming-parent-review/v1` object with:
`accepted: true`, distinct nonempty `implementation_run_id`/`reviewer_run_id`,
`binding` copied exactly from gates.json, `final_state_digest` and `gates_digest`
(SHA-256 of compact sorted-key JSON for `final_state` and `gates`),
`baseline_index_digest` equal to gates.json's `index_digest`,
`review_artifact` and `snapshot` objects each containing canonical absolute
`path` and byte `sha256`, and
`preservation: {"scope":true,"body":true,"abi":true,"range":true,"index":true}`.
These are explicit parent authority assertions, never inferred from review prose.
The actual accepted reviewer artifact must be retained. Snapshot is the cleaner's
`{"files":[...],"frozen":{...}}` manifest: each transaction path has `path`,
`exists`, and (when present) `sha256`/`mode`; preserved copies live beneath the
manifest directory at those relative paths. It must cover every captured
binding/source path plus old definition and destination, including absence.
`frozen` binds report, reviewed paths and raw `.git/index` SHA-256. Ingestion
checks retained snapshot bytes/modes/absence, not just the manifest hash.

Trust is the local repository runtime and supervising parent, like local
application attestations—not remote authentication or protection from malicious
local writers. Preapply body/ABI/range/scope preservation is parent/reviewer
attested against the real snapshot; prepared naming provenance does not contain
old source bytes. Native gates prove execution/current scope/exactness and
post-start index preservation. Review ingestion rechecks readiness and diff
hygiene; final verification repeats these and all existing naming validators,
then rechecks report/state/index and retained evidence. Frozen report bytes and
authored provenance are never rewritten; receipts overlay only a selected-row
copy after original provenance. Duplicate/unknown new-route JSON fields and
in-row/external ambiguity reject. Omitted `--post-apply-receipts` stays legacy.
Failures retain native evidence and return rollback ownership to cleaner/parent,
never restore HEAD or recover derived state automatically.

Reviewed type applications are concern-isolated and atomic. The disposable reverse index only supplies leads; `prepare` requires a separately reviewed, live-fingerprinted candidate artifact with resolved representation and semantics plus two independent observations. On a dirty worktree, the request must include the exact adopted baseline digest printed by the preflight error/workflow. `run` restricts writes to manifest-owned paths, executes the recorded checks, writes immutable structured receipts, and rolls all changes back on failure:

```sh
bin/type-audit account out/reviews/type-account.json
bin/type-audit validate-account out/reviews/type-account.json
bin/type-audit baseline  # copy digest into adopted_baseline when adopted=true
bin/type-audit prepare out/reviews/type-request.json out/reviews/type-manifest.json
bin/type-audit run out/reviews/type-manifest.json out/reviews/type-changes.json out/reviews/type-application.json
bin/type-audit verify out/reviews/type-application.json --expected-application-digest DIGEST
```

The changes file is a JSON object mapping each allowed repo-relative file to its complete replacement text. Retain the application digest from the `run` output in a trusted external record; do not derive the expected value from the application file being verified. Shared preparation requires two externally pinned reviewed private envelopes with identical representation and semantic contracts; target-address-bearing contracts are rejected. Integrity-only pins cannot authorize shared preparation.

For macro applications, parent review, and revalidation, use the canonical
[macro resolution workflow](specs/MACROS.md#reviewed-application). The following
sections document type transactions.

For context-bearing native runs add `--implementation-run-id IMPLEMENTATION_RUN`
to type `run`. The application retains `review_context` bound before edits,
around every gate and at publication: owner-derived source/header/config/binary,
resolver catalog (including absence), tooling, modes, adopted baseline, index,
environment digest, native tool resolution and Ninja build transitions. Only
explicit text replacements may change authoritative inputs; original permissions
are preserved. Toolchain overrides, missing configured compiler installs and
unexpected drift reject. New runs require explicit-source, companion-free owners
and Ninja; no installation or index recovery is authorized. Context-bearing
integrity replay rechecks live closure; omission preserves the legacy route.
Context is not a reviewer attestation and cannot establish final acceptance.

Type `verify` remains integrity/native-gate verification, not independent
final acceptance. Exact asm-diff/byte-match receipts require their native JSON
schema, positive exact/byte-match semantics and selected source/function/address,
target-owned binary and size consistency; exit-zero warnings or malformed payloads
fail. Stdout JSON is retained separately from stderr and revalidated on replay.
Splat/build use their native plain-output exit contract. Existing partial-match
baselines remain exact-output pins. Parent-review envelopes add independent private acceptance below; shared
preparation consumes reviewed envelopes, not legacy integrity pins.

Production macro/type application and revalidation checks share naming
postapply's bounded process owner: 120 seconds and 2 MiB of native output per
command. Timeout and overflow produce failed receipts (124 and 125). Abrupt
owner death still requires owned-PRE reconciliation, not only process cleanup.

Type `run` and `revalidate` acquire the same nonblocking writer lease as macro
transactions and retain it through native checks, publication and rollback.
Contention rejects before source mutation. The persistent private lock under
`out/reviews/evidence/transaction.lock` is not a stale-PID file to delete; neither
its presence nor successful acquisition proves prior worker termination. Manual
editors and other nonparticipating commands are outside this cooperative contract.
Read-only verification/inspection acquire no lease. See
[harness ownership](specs/HARNESS.md) for identity-loss and recovery limitations.

Before modifying sources, macro/type runs persist private v3 recovery records at
`out/reviews/evidence/{macro,type}-recovery-<nonce>.json`. They bind root,
manifest/run/publication, PRE images, exact prepared POST inode/mode, private
staging and reserved PRE/POST quarantines. Images require the same filesystem and
native no-replace support; new-file mode reflects the actual umask.
With Git-backed snapshots, v3 also archives untouched workspace PRE content and
metadata plus exact index bytes/state. Inspection reports scoped drift and index
locks without printing those bytes; unavailable guards are explicit. Matching
observations are non-atomic and grant no restoration authority. Historical v1/v2
records remain inspectable without being upgraded to these safeguards.
Keep its digest independently when supervising a run. Inspect without mutation:

```sh
bin/type-audit inspect-recovery out/reviews/evidence/type-recovery-NONCE.json --expected-recovery-digest INDEPENDENT_PIN
```

The summary omits PRE source images and reports drift and publication presence;
success is not restoration readiness. A discovered record or PID alone does not
authorize restoration: guarded restoration commands and
unrelated workspace/Git-index recovery remain unfinished. Macro invocation belongs
to [MACROS.md](specs/MACROS.md); see
[harness recovery contracts](specs/HARNESS.md).

### Type parent review and final verification

Use `bin/type-audit` with the same owner throughout:

```sh
bin/type-audit review APPLICATION out/reviews/evidence/reviewed.json --parent-attestation PARENT_JSON --expected-application-digest APPLICATION_DIGEST
# Retain the printed envelope digest externally before final verification.
bin/type-audit final-verify out/reviews/evidence/reviewed.json --expected-envelope-digest ENVELOPE_DIGEST
```

The supervising parent authors a closed `bof3.type-parent-review/v1`
object after a distinct reviewer inspects the actual
application and native receipts. Fields are `schema`, `accepted: true`, distinct
nonempty `parent_run_id`, `implementation_run_id`, `reviewer_run_id`,
`review_artifact: {path, sha256}` (canonical absolute retained nonempty reviewer
bytes), `binding`, and `preservation`. Implementation ID must match the real
captured run. Preservation has exactly `scope`, `body`, `abi`, `range`, `index`,
`adopted_baseline`, all literal `true`, explicitly attesting inspection against
the adopted baseline; no verdict is inferred from reviewer prose.

Binding has exactly `application_digest`, `application_proof_digest` (entire
application including original attestation), `manifest_digest`, `request_digest`,
`review_context_digest`, `post_state_digest`, `native_receipts_digest` (full
receipts array), `adopted_baseline_digest` (context's full adopted baseline).
Digests use SHA-256 of compact sorted-key JSON, except retained artifact hashes
which cover bytes. No missing context/legacy omitted-ID proof is promotable.

The closed `bof3.type-reviewed-application/v1` envelope contains
`schema`, original `application`, `parent_review`, and `digest` over the other
three fields. Final verification requires the externally retained digest, checks
original owner proofs/receipts and captured current closure, retained reviewer
bytes and unrelated adopted workspace preservation. It never rebinds historical
proofs, reruns gates or reapplies edits. Identical read-only replay is valid;
reuse for another application, owner, context or changed state rejects. Trust is
the supervising parent and local native runtime, not protection against a
malicious local writer. Shared input pins use exactly `path`, `target`, and
`expected_envelope_digest`. Paths remain canonical under `out/reviews`;
retain the envelope digest externally, never derive it from the supplied file.
Preparation verifies each original reviewed private envelope against current
state and retains its full bytes/hash and application in the shared manifest.
Distinct target/path/digest, private/exact-wrapper, equal-contract and shared
owner/dependency checks remain required. Legacy `expected_application_digest`
shared pins reject; standalone `verify` remains integrity/native-gate only.

Shared parent acceptance is supported only for the two-fresh-revalidation branch
below. Original private application envelopes alone remain preparation-only;
standalone private final verification still requires its own current closure.

Owner Python APIs expose `revalidate_application` and
`verify_revalidation` for independently reviewed private envelopes. Omitted `intervening` retains the
still-current-only route. Supply the
external envelope digest, a new distinct `execution_run_id` and exact current
`adopted_baseline` digest. Revalidation runs new native gates without reapplying
edits and returns separate `bof3.type-application-revalidation/v1`
evidence; replay requires its externally retained digest. `checked: true` is not
independent acceptance and cannot authorize shared promotion. Original proof
closure must remain current unless the exact reviewed sequential transition below
explains it; unevidenced graph drift rejects. Unexpected gate writes require explicit recovery and are not overwritten.
Fresh parent acceptance uses owner `review_revalidation(root, record, parent,
expected_revalidation_digest)` and externally pinned
`verify_reviewed_revalidation(root, envelope, expected_envelope_digest)` APIs.
The closed parent object uses `bof3.type-revalidation-parent-review/v1`,
the same identity/artifact/preservation fields as application review,
and binds `implementation_run_id` to the new check execution. The reviewer run
must differ from all original review identities and use a fresh retained artifact;
the supervising parent may remain the same. Binding has exactly
`revalidation_digest`, `prerequisite_envelope_digest`, `manifest_digest` (full
revalidation manifest), `review_context_digest`, `pre_state_digest`,
`native_receipts_digest` and `adopted_baseline_digest` (full context baseline).
Digests use compact sorted-key JSON as above. The resulting closed
`bof3.type-reviewed-revalidation/v1` contains `schema`,
`revalidation`, `parent_review` and `digest`. Retain its digest externally.
Replay checks both original acceptance and new evidence/current closure without
rerunning gates, publishing files or rewriting proofs. These envelopes can now supply the shared PRE prerequisite pins below; they are
not shared POST acceptance. For intervening private mutations, pass an ordered `intervening` list of closed
`{envelope, expected_envelope_digest}` objects from the same owner. Each original
and intervening private envelope requires its externally retained pin and original
parent acceptance. Retained history is checked without rebinding; consecutive
original POST/intervening PRE and POST/current states must agree, including native
build closure, tools/environment/index and unrelated adopted workspace. Only the
reviewed transaction's changed paths explain authoritative drift; original owned
POST bytes/modes remain intact. Manifest-specific retained evidence membership is
validated separately, not treated as native input mutation authority. Build changes
are accepted only inside original owner `bin/build` receipt transitions, never in
gaps. New check-only gates must leave this final state unchanged. Both results can
then receive distinct fresh executions and fresh parent acceptance at that state.
The revalidation retains every intervening envelope and external pin; replay
rechecks the full sequence and nested evidence. Explicit adoption alone never
permits drift. Shared historical PRE/current POST validation uses the bounded transition below. For distinct-target private sequences, type `run_transaction`
Python calls (CLI `run --participating-targets TARGET TARGET`) must declare the same sorted `participating_targets` list before either
execution (one or two canonical configured targets, including all mutation targets;
requires `implementation_run_id`). This freezes additional evidence scope only:
each participant's manifest-derived binary, Splat, reviewed/maps/include closure is
captured without adding writable paths or checks for that participant. Omission
retains selected-target capture; naming is unchanged. Revalidation inherits the
original set, never expands it; missing/mismatched historical scope rejects.
Both private results still require their own fresh native gates and parent review.
Other shared acceptance branches remain guarded.

The same owner commands transport these APIs directly:

```sh
bin/type-audit revalidate PRIVATE_ENVELOPE out/reviews/evidence/check.json --expected-envelope-digest PRIVATE_PIN --execution-run-id FRESH_RUN --adopted-baseline CURRENT_BASELINE --intervening ORDERED_PINS_JSON
bin/type-audit verify-revalidation out/reviews/evidence/check.json --expected-revalidation-digest CHECK_PIN
bin/type-audit review-revalidation out/reviews/evidence/check.json out/reviews/evidence/fresh.json --expected-revalidation-digest CHECK_PIN --parent-attestation FRESH_PARENT_JSON
bin/type-audit final-verify-revalidation out/reviews/evidence/fresh.json --expected-envelope-digest FRESH_PIN
```

Omit `--intervening` for still-current private results; otherwise its JSON is the
ordered `{envelope, expected_envelope_digest}` list above, with full retained
objects and externally retained pins. Outputs remain canonical repo-relative
paths under `out/reviews/evidence`; check and fresh review outputs must not exist. Use `bin/type-audit baseline`
(the shared workspace owner) for current adoption, never to excuse drift. Revalidation
inherits participation; there is no revalidation scope-expansion flag. Keep fresh
checks, independent review and parent acceptance separate. These commands neither
schedule steps nor infer approval or recover the index.

`bin/rev-query mission TARGET@0xADDRESS` composes a single-function lifting
brief (metrics, callers/callees, duplicate group, SDK callees, and risk flags) —
a starting point for a directly requested lift using `$bof3-re`.
Source changes still require target-qualified evidence and applicable checks;
Git writes and publication require explicit authorization.

Addresses are target-qualified where identity matters; overlapping addresses
in different images never share query results.

### 5. Lift and iterate

```sh
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit the metadata-resolved lift source and adjacent target evidence
bin/asm-diff TARGET@0xADDRESS --detail normal
bin/byte-match TARGET@0xADDRESS
```

`m2ctx` materializes target-owned declarations. `m2c` creates a complete seed,
not reviewed C. `asm-diff` prints a bounded diagnostic and keeps the full patch
under `out/asm-diff/`; `byte-match` is the acceptance check. For a function with
an out-of-image companion call, run `bin/companion-check TARGET@0xADDRESS` first;
it exits nonzero until static-call identity, companion boundary/map, reviewed ABI,
and matching caller declaration evidence are all present.

When readable semantics are credible but code shape differs:

```sh
bin/flag-search TARGET@0xADDRESS
bin/permute TARGET@0xADDRESS --time-limit 60 --quiet -j N
```

`bin/data-scan [TARGET...]` lists unlabeled in-image data regions referenced
by lifted functions (BSS globals vs file-backed tables/strings, with reference
counts) — the data-labeling and table-extraction backlog. Each region links any
indexed type candidates with their evidence class, status, width, and blocker;
this linkage is triage evidence, never a promoted layout. `--all` widens to
unlifted functions, `--json` for tooling.

`flag-search` suggests compiler flags from known profiles. `permute` searches
source shapes in a disposable workspace. `promote` validates the canonical
source but never edits source, maps, or layouts.

### Share a decomp.me scratch

```sh
bin/scratchpad preview TARGET@0xADDRESS
bin/scratchpad share TARGET@0xADDRESS
```

`preview` is local-only and prints the exact payload. `share` creates a public,
unclaimed decomp.me PS1 scratch with the target assembly, the authored C body,
and minimal generated target declarations/context; it prints the resulting URL.
Sharing is opt-in and must never include user media, private assets,
credentials, or unreviewed `out/` candidates. It accepts only a reviewed Splat
function boundary with an authored source file; missing lifting ABI/call evidence
does not itself make that function unshareable. It fails closed for a lift that
uses ignored PsyQ declarations; add a reviewed public declaration boundary
before sharing that class of function. It defaults to the canonical local
`gcc-2.7.2-psx`, mapped to decomp.me's `gcc2.7.2-psx` compiler ID; it does not
change a local compiler/object selection or constitute matching evidence.

To try a catalog compiler instead of the canonical one, pass `--compiler` with
a catalog ID, e.g. `bin/flag-search TARGET@0xADDRESS --compiler gcc-2.8.0-psx`.
Its output is diagnostic only: a non-exact result never retains an object
override, and even a fresh exact result needs a reviewed
`BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` entry in `config/compiler/object-flags.cmake`
before the build selects it.

### 6. Promote duplicate knowledge

Follow the evidence gate and ownership model in
[function matching: Exact duplicate groups](agents/matching.md#exact-duplicate-groups).
It is the normative duplicate-promotion procedure; every wrapper remains
address-owned and independently validated.

### 7. Audit and hand off

```sh
bin/build TARGET@0xADDRESS
bin/build TARGET
bin/decomp-status TARGET --detail normal
just check
git diff --check
```

`build` compiles authored objects; it does not reconstruct a complete image.
`decomp-status --detail minimal` prints totals, `normal` adds target totals and
invalid details, and `full` prints every function. `just check` runs repository
tests, lint, maps, and a cached repository audit of retained lifts; it is not
acceptance evidence for an individual lift.

On a cold or partially invalidated cache, `decomp-status` batch-builds all
valid cache-miss objects per owning target in a single CMake invocation, then
compares each individually. An all-cache-hit target issues no build command.
`--no-cache` bypasses disposable audit summaries but still batch-builds selected
valid misses; it is for diagnosis, not lift acceptance. Cache rows are never
used for acceptance: immediately before accepting a lift, run live
`asm-diff`, `byte-match`, `companion-check` where relevant, `splat`, and
`symbols check`.

For iteration, select the affected target instead of repeatedly auditing every
target; retain repository-wide handoff checks. `--detail` only changes displayed
output, not validation work. Profile warm-cache and `--no-cache` runs separately:

```sh
PYTHONPATH=tools/python .venv/bin/python -m cProfile -o /tmp/bof3-status.prof -m harness.commands.decomp_status TARGET --no-cache --detail minimal
.venv/bin/python -c 'import pstats; pstats.Stats("/tmp/bof3-status.prof").strip_dirs().sort_stats("cumulative").print_stats(25)'
```

`--no-cache` bypasses status summaries, not native build caches. Manifest reuse
stays within the current operation; comparison reloads ownership after building.
Each manifest-validation pass rechecks every claim's contents and canonical path.

## Command ownership

| Command | Why it exists | Primary artifacts |
| --- | --- | --- |
| `bin/bof3-disk` | inspect/extract original disc files | chosen output |
| `bin/emi-ex` | list, extract, or explicitly repack EMI archives | chosen output |
| `bin/str-media` | inspect, validate, or convert STR media | chosen output |
| `bin/emi-target` | preview/create one EMI target | only with `--apply` |
| `bin/companion-check` | gate a lift through a declared EMI companion call | JSON readiness report |
| `bin/build` | compile all, one target, or one function | `build/` |
| `bin/splat` | regenerate reviewed segment output | `out/splat/` |
| `bin/spimdisasm` | disassemble MIPS images directly | terminal output or explicit destination |
| `bin/symbols` | map check/normalize, bindings, PsyQ import/bindings/report | explicit subcommand |
| `bin/rizin` | pinned local Rizin analyzer | terminal output only |
| `bin/rz-project` | isolated Rizin analyze/status/open | `out/reverse/snapshots/` on analyze |
| `bin/index` | rebuild the fresh cross-target query cache | `out/index/` |
| `bin/rev-query`, `bin/analysis-readiness` | query fresh indexed evidence and bounded aggregate readiness | none |
| `bin/naming-audit`, `bin/type-audit`, `bin/macro-audit` | validate concern-owned evidence and run explicitly prepared atomic transactions | review artifacts and receipts under the chosen `out/` paths |
| `bin/m2ctx`, `bin/m2c` | generate target context and C seed | `out/` or `-o` |
| `bin/asm-diff`, `bin/byte-match` | compare one authored lift | `out/asm-diff/`, `out/matching/`, `out/bindings/`, `build/` |
| `bin/flag-search` | rank known compiler flag profiles | report plus `out/matching/` baseline |
| `bin/permute` | bounded source-shape search | `out/permuter/` |
| `bin/promote` | validate canonical candidate | generated comparison only |
| `bin/decomp-status` | audit exact/partial/invalid lifts | `out/matching/`; full JSON with `-o` |
| `bin/psyq-import` | stage PsyQ build headers | explicit destination |
| `bin/harness psyq` | permanent narrow scan/calls/proposal PsyQ signature-evidence adapter | `out/psyq/` |

The shared panel-task implementation template lives at `src/shared/ui/panel_task.inc`; target-local wrappers compile it and retain symbol/address ownership.

`bin/cc`, `as`, `ld`, `ar`, `nm`, `objcopy`, `objdump`, `ranlib`, `strip`, and
`maspsx` are build adapters. Workflow users should call `bin/build` and the
matching commands instead of invoking these adapters directly.

See [function matching](agents/matching.md) for C iteration rules,
[build analysis evidence](#3-build-analysis-evidence) for analyzer contracts, and
[project context](agents/project-context.md) for ownership.

## Plans

```sh
bin/plans list
bin/plans status autonomous-bof3-decompilation.md
bin/plans consolidate /absolute/review.json
# Only after independent semantic review of these exact bytes:
bin/plans consolidate /absolute/review.json --apply --backup-dir /absolute/new-recovery
```

Omit the status filename only with exactly one plan. Lists/status read persistent
Markdown, not session history. Consolidation previews by default; apply requires
a fresh external recovery directory and hash-bound reviewed candidate/mappings.
See [plan authoring](agents/plan-authoring.md) and `$plans`. These commands
neither infer semantic completion nor execute the plan's domain commands.


### Bounded shared PRE preparation

The existing shared pin fields also accept `bof3.type-reviewed-revalidation/v1`
envelopes. This branch requires exactly two fresh accepted private
revalidations with distinct execution IDs and targets, identical explicit two-target
capture scope, common current native inputs/environment/index/build/workspace,
and all existing private contract/ownership/exact-wrapper constraints. Mixed fresh
and original application envelopes reject. Complete nested envelopes and external
pins remain frozen in the manifest, alongside `shared_pre` state/build/baseline.
`run` requires an implementation ID and the identical `participating_targets`
argument (CLI `--participating-targets`), rederives the manifest and compares captured PRE before edits or
gates. Omitting these bindings cannot downgrade the fresh branch to legacy execution.
Canonical absolute parent reviewer artifacts are checked by the parent evidence
validator, not interpreted as repository-relative native inputs; other absolute
evidence is not silently excluded. Existing original-envelope preparation is
unchanged and cannot receive shared acceptance.

### Bounded shared POST acceptance

For the fresh branch, use the existing owner `review` and `final-verify` commands
and parent schema above, with a new shared implementation and independent reviewer
not reused from any private prerequisite or fresh revalidation. Retain a new
reviewer artifact and externally pin the new shared envelope digest. Replay checks
original/intervening private histories, original and fresh parent reviews, retained
native receipts/artifacts and the identical frozen common PRE; it separately
validates the shared authorized delta, new native gates, current POST/context and
unrelated adopted workspace. It never compares old private closure to intended
shared POST, rewrites proofs, reruns gates or substitutes current facts for missing
history. Existing contracts, exact wrappers and ownership restrictions remain at
preparation; participation is capture scope, not write authority. Local parent/runtime
trust is not cryptographic authentication. This bounded tooling integration was independently accepted (see the canonical
plan); it is not live BOF3 type/template promotion or full S3.4 closure.

### Naming terminal acceptance (read-only)

`validate` and exact capabilities prove structural integrity, not independently
accepted exhaustion/current no-op. The bounded terminal owner supports only
existing capability-backed exhausted rows; proposals/applications keep their
existing owners. No report, capability, receipt or identity is rewritten.

```sh
bin/naming-audit terminal-verify TARGET REPORT --transaction data:NAME --parent-attestation PARENT_JSON --expected-parent-digest EXTERNALLY_RETAINED_PIN --evidence-root CANONICAL_ROOT
```

Omit the root only for default-root evidence. Preparation calls the read-only
Python `harness.naming.terminal.terminal_binding(root,target,report,transaction)`
under the same evidence context. This returns integrity/scope binding, **not
acceptance**: exact canonical target/selector/transaction/report bytes and mode,
row/capability digests, retained namespace evidence digest, missing naming fact,
and owner-derived current repository/scope/tooling digest. Current inventory,
index/manifest freshness, row/receipt and capability validators must pass. Derived
`open`/`unavailable` evidence rejects. Unrelated report-row failures are returned
separately as `full_report.blocker`, not silently counted as complete.

The supervising parent supplies a closed `bof3.naming-terminal-parent-review/v1`
object with exactly `schema`, `accepted:true`, `parent_run_id`,
`evidence_preparation_run_id`, `reviewer_run_id`, `binding`,
`preparation_artifact`, `review_artifact`, `ladder_exhausted:true`,
`unresolved_leads:[]`, `ceiling_rationale`. Three run IDs must be nonempty and
distinct. The expected parent digest is SHA-256 of compact sorted-key JSON,
retained externally by the caller, never inferred from the supplied file.

Artifact references have exactly canonical absolute `path` and byte `sha256`;
files must be retained, nonempty and symlink-free. Separate preparation JSON has
exactly `schema: bof3.naming-terminal-preparation/v1`,
`evidence_preparation_run_id`, `binding`. Semantic reviewer JSON has exactly
`schema: bof3.naming-terminal-semantic-review/v1`, `reviewer_run_id`, `binding`,
`ladder_exhausted`, `unresolved_leads`, `ceiling_rationale`. All shared values must
match the parent. Rationale has exactly `missing_fact` (from binding), nonempty
`explanation` addressing the static ceiling despite that missing fact, and
nonempty `evidence` (retained path/hash references). False ladder, executable open
leads, contrary reviewer records, missing evidence, self-review and stale state
reject. The parent/reviewer must inspect the referenced evidence: missing a second
consumer alone does not establish a static ceiling. This validator does not infer
semantic truth from prose or discover omitted leads.

Trust is explicit local supervising-parent attribution, not authentication of
historical collection origins. Preparation ID names the actual native run assembling
or inspecting this bound submission; old collection journals do not record it.
The separate preparation artifact attributes that run without inventing a mutation
implementation identity. No production attestation is supplied by tooling/tests.
Current `data:D_80096994` remains blocked by its independent reviews' false ladder
and unresolved leads; historical structural exhaustion cannot advance it.

Success returns `selected_row_accepted:true`, separate `full_report` status, and
`production_complete:false`. The future autonomous consumer must use this gate
for capability exhaustion/no-op, not structural validation. Successful full-report
validation with `complete:true` remains an additional mandatory production gate;
row acceptance never replaces it or accepts other rows. No loop scheduler, native
cancellation recovery, identity authorization or full-target completion is added.
