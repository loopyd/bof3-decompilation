# Macro discovery and ranking

Contents: [admission](#admission), [queries](#query-and-inspect),
[ranking](#human-value-ranking), [review](#semantic-review).

## Admission

Prioritize original bytes, reviewed ranges, manifests and source metadata over
analyzer leads. Require caller-supplied positive instruction floor, pool size and
top-N. Admit four or more disjoint physical uses; sites may share a function.
Rank largest blocks first, then human value. Require independently exact private
proofs per target, two or more targets for shared promotion and consumer review.

Discover constants/accessors at three occurrences; statement windows at two
matching three-statement windows of twelve tokens each; exact groups at two
nontrivial members; near-duplicates at two compatible functions of at least three
instructions. Admit leads only. Mixed byte/token/occurrence scores cannot prove
largest-first ranking. Keep generated candidates blocked, without automatic
application or a durable attempt ledger.

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

Omit optional NAME/target filters as needed. `--limit` defaults to 20; zero means
all, not an attempt budget. Stale source/binary/index blocks discovery. Parent
alone refreshes at safe checkpoints.

Index manifest-owned source/support/headers, shared helpers and templates with
bodies/parameters/hashes/conditions/provenance/restrictions/lexical uses. Exclude
generated PsyQ bindings. Historical labels grant no register pins, clobbers, empty
asm, aliases or no-op aids. Preserve address bindings; requeue consumers for clean C.

Require reviewed-byte identity to binary and equal instruction/CFG/call/data shape
for near-duplicates. Parameterize only immediate/address deltas; keep branch
displacement exact. Exclude generated/trivial/data entries and boundary disagreement.

Retain block opcode/register fields and branch displacements; parameterize explicit
immediates/addresses. Require reviewed function range/hash and valid PS-X payload.
Exclude SDK/generated/trivial/data/boundary-disagreement entries. Keep sequences
within functions. Select the longest prefix with four disjoint occurrences;
suppress contained alternatives with identical site ordering. This does not
enumerate all register-renamed/CFG-equivalent abstractions. Retain target/function,
function/source hash, byte offset, start/exclusive end, block hash and parameters
per member. Source mapping, transfer/delay-slot context, liveness and semantics
remain unproven.

Target filters only view global groups; preserve fingerprints and write authority.
`bof3.macro-block-opportunities/v1` carries floor, four-use minimum, ordering/count
and null `attempt_budget`. Account blocks only with explicit floor and pinned
`block_discovery`; validate the same scope. A stricter floor cannot rebind a lost ID.

Use `describe` for `bof3.macro-resolution/v1`, global membership/fingerprint and
focused lexical context. Use `impact` for `bof3.macro-impact/v1`, indexed uses and
transitive references through cycles/conditions. Include other parameter/macro
consumers in writable files. Lexical names are not expansions; review include
order, conditional selection, `#undef`, unindexed/computed names and token pasting.
Impact remains `complete:false`, without `all_use_sites` proof or wider writes.

Account every scoped opportunity exactly once in `bof3.macro-candidate-account/v1`
with status/reason/membership/candidate and input fingerprints. `complete` means
accounted inventory, not accepted cleanup. Revalidation rejects drift.

## Human-value ranking

```sh
bin/harness macros rank-input --min-instructions FLOOR --target TARGET --pool-size POOL --top-n TOP --require-source > REQUEST
bin/harness macros rank REQUEST RANKING --expected-pool-digest POOL_PIN
bin/harness macros validate-ranking RANKING --expected-ranking-digest RANKING_PIN
```

Freeze `pool_digest` externally before assessment. Bind `bof3.macro-ranking-request/v1`
to exact policy/population/largest-first pool and positive `top_n <= pool_size`.
Apply optional `require_source` globally; `population.source_missing` counts filter
exclusions, not all unlifted code.

Edit only `assessor`/`assessments`; inspect every member C/original block/existing
template. Each candidate needs exact ID/fingerprint, `try|defer|reject`,
rationale and five criteria: `coherent_operation`, `meaningful_parameters`,
`clear_call_sites`, `reduced_repetition`, `maintainability`. Every criterion has
nonempty evidence and `promising|poor|unknown`. For try, require all promising and
unique contiguous positive priorities from 1; otherwise use null priority. Reject
requires one poor criterion.

`rank` reconstructs the pool and validates its external pin/all assessments.
`bof3.macro-ranking/v1` retains decisions and at most top-N human-priority tries;
mark extras `budget_deferred`. Use an empty queue when none qualify. Keep global
members `selected_for_review`, not accepted, and pin the ranking externally.
Reject unknown fields, stale/incomplete assessments or changed pins.

Retain `safe_application_count:0` and `execution_started:false`. Caller cumulative
attempt accounting survives repeated ranking, which grants no native recovery.

## Semantic review

Inspect instructions, C89, mapping, every use site and existing abstractions.
Require coherent operations, meaningful parameters, clear calls and maintainable
definitions. Reject incidental similarity and hidden flow/evaluation hazards.
Preserve useful macros; do not re-extract for receipts. Route grouping to its owner.
Keep wrappers/maps/declarations/boundaries target-local and shared bodies address-free.

Resolve eight guards with explicit evidence: `evaluation_count`, `side_effects`,
`integer_promotions`, `precedence`, `lvalue`, `volatile`, `aliasing`, `control_flow`.
Each `resolved|not_applicable`, nonempty evidence. Independent observations
`parameter_mapping`, `all_use_sites`; blocks additionally require `human_value`.
Empty/unresolved/unaccepted review blocks prepare. AI ranking and byte equality
are not independent readability/semantic acceptance or safety for future arguments.

Fixed-address macros and named bindings compile differently; neither always wins.
Preserve measured PRE, relocation form, pointer-cell/pointee qualifiers and full
register/frame effects. A historical exact macro is not new extraction authority.
