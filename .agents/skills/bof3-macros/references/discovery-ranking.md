# Macro discovery and ranking

Contents: [admission](#admission), [queries](#query-and-inspect),
[ranking](#human-value-ranking), [review](#semantic-review).

## Admission

Original bytes, reviewed ranges, manifests and source metadata outrank analyzer
leads. Require caller's positive instruction floor, pool size and top-N; no default
floor. At least four distinct non-overlapping physical uses of proposed shape;
sites may share a function. Rank largest blocks first, then human value. Four-use
eligibility is separate from independently exact private proofs per declared target
and at least two targets for shared promotion. Neither waives consumer review.

Lexical constants/accessors need three occurrences; statement windows need two
matching three-statement windows, twelve tokens each; exact groups need two
nontrivial members; near-duplicates need two compatible functions, three instructions
minimum. These are raw lead thresholds, not extraction admission. Mixed byte/token/
occurrence scores do not establish largest-instruction-first ranking. All generated
candidates remain blocked; no automatic applications or durable attempt ledger.

## Query and inspect

```sh
bin/harness analysis query macros NAME --target TARGET
bin/harness analysis query macro-uses NAME --target TARGET
bin/harness analysis query macro-opportunities --target TARGET
bin/harness analysis query near-duplicates --target TARGET
bin/harness macros blocks --min-instructions FLOOR --target TARGET --limit COUNT
bin/harness macros account ACCOUNT --min-instructions FLOOR
bin/harness macros validate-account ACCOUNT
bin/harness macros describe ID --min-instructions FLOOR --target TARGET
bin/harness macros impact DEFINITION_ID
```

Optional NAME/target filters may be omitted. `--limit` defaults to 20, zero means all;
output limit is not attempt budget. Discovery never refreshes index. Stale source/
binary/index evidence blocks. Parent refresh at safe checkpoint only.

Index includes manifest-owned source/support/headers, shared helpers and templates,
retaining bodies/parameters/hashes/conditions/provenance/restrictions/lexical uses.
Generated PsyQ bindings are noncandidates. Historical helper labels never authorize
register pins, clobbers, empty asm, aliases or no-op aids; preserve address bindings
and requeue affected consumers for clean-C matching.

Near-duplicates require reviewed-byte identity to binary, equal instruction/CFG/
call/data-reference shape; only immediate/address deltas qualify, branch displacement
stays exact. Exclude generated/trivial/data entries and boundary disagreement.

Blocks retain opcode/register fields and branch displacements with explicit
immediate/address parameters. Require reviewed function range/hash and valid PS-X
payload; exclude SDK/generated/trivial/data/boundary-disagreement entries. Sequences
stay within functions. Longest prefix admitting four disjoint occurrences wins;
contained alternatives with same site ordering suppressed. No claim of all possible
register-renamed or CFG-equivalent abstractions. Each member retains target/function,
function/source hash, byte offset, start/exclusive end, block hash and parameters.
Source mapping, transfer/delay-slot context, liveness and semantics remain unproven.

Target filter is view of global group, not smaller fingerprint or external write
authority. `bof3.macro-block-opportunities/v1` carries floor, four-use minimum,
ordering/count and null `attempt_budget`. Account includes blocks only with explicit
floor, pinning `block_discovery`; validation replays same scope. Stricter floor
cannot rebind disappeared ID.

`describe` yields `bof3.macro-resolution/v1`, global membership/fingerprint and
focused lexical context. `impact` yields `bof3.macro-impact/v1`, indexed uses and
transitive definition references through cycles/conditions. Candidate members are
not full consumers; other parameters or macros in writable file add consumers.
Lexical names are not expansions. Include order, conditional selection, `#undef`,
unindexed/computed names and token pasting require review. Impact stays
`complete:false`; it never proves `all_use_sites` or grants wider writes.

`bof3.macro-candidate-account/v1` covers each current scoped opportunity exactly
once with status/reason/membership/candidate and input fingerprints. `complete`
means inventory accounted, not cleanup accepted. Validation rederives and rejects drift.

## Human-value ranking

```sh
bin/harness macros rank-input --min-instructions FLOOR --target TARGET --pool-size POOL --top-n TOP --require-source > REQUEST
bin/harness macros rank REQUEST RANKING --expected-pool-digest POOL_PIN
bin/harness macros validate-ranking RANKING --expected-ranking-digest RANKING_PIN
```

Freeze `pool_digest` externally before assessor. `bof3.macro-ranking-request/v1`
binds exact policy/population/largest-first pool. Positive `top_n <= pool_size`.
Optional `require_source` covers every global member, not focused target only;
`population.source_missing` counts filter exclusions, not all unlifted code.

Assessor edits only `assessor`/`assessments`; inspect every member C/original block/
existing template. Each candidate needs exact ID/fingerprint, `try|defer|reject`,
rationale and five criteria: `coherent_operation`, `meaningful_parameters`,
`clear_call_sites`, `reduced_repetition`, `maintainability`. Every criterion has
nonempty evidence and `promising|poor|unknown`. Try needs all promising and unique
contiguous positive priority from 1; others null priority; reject needs one poor.

`rank` reconstructs machine pool, validates external pin and all assessments.
`bof3.macro-ranking/v1` retains decisions, at most top-N tries by human priority;
extra tries `budget_deferred`. Empty queue valid when none worthwhile. Members stay
global, `selected_for_review`, never accepted. Retain ranking digest externally.
Unknown fields, stale/incomplete assessments or changed pins reject.

`blocks`, `rank-input`, `rank`, `validate-ranking --work-deadline` use original
absolute monotonic cutoff. Cooperative expiry is failure, not partial success;
late output may remain. No preemption guarantee, budget renewal or native recovery.
`safe_application_count:0`/`execution_started:false` remain. Caller owns cumulative
attempt accounting; repeated ranking never resets it.

## Semantic review

Inspect instructions, proposed C89, mapping, every use site and existing abstractions.
Prefer coherent operation, few meaningful parameters, clear call sites and maintainable
definition. Reject incidental similarity, hidden flow/evaluation hazards. Do not
unshare/re-extract useful macros for receipts. File grouping is separate owner scope.
Keep wrappers/maps/declarations/boundaries target-local; shared bodies address-free.

Resolve eight guards with explicit evidence: `evaluation_count`, `side_effects`,
`integer_promotions`, `precedence`, `lvalue`, `volatile`, `aliasing`, `control_flow`.
Each `resolved|not_applicable`, nonempty evidence. Independent observations
`parameter_mapping`, `all_use_sites`; blocks additionally require `human_value`.
Empty/unresolved/unaccepted review blocks prepare. AI ranking and byte equality
are not independent readability/semantic acceptance or safety for future arguments.

Fixed-address macros and named bindings compile differently; neither always wins.
Preserve measured PRE, relocation form, pointer-cell/pointee qualifiers and full
register/frame effects. A historical exact macro is not new extraction authority.
