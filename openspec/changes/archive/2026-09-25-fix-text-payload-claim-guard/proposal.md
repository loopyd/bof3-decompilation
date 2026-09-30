# Proposal

## Why

`just check` fails at `tools/rust/bof3-text/tests/text_payload_removals.rs` in
`no_current_claim_contradicts_the_candidate_count`:

```
out/text-format/validation-summary.md:20: claims 32 heuristic instances where the artifacts report 0
```

That guard exists for a real reason — its own comment records that a `**21** candidate runs`
claim once survived an earlier, weaker version. It scans every `out/text-format/*.md`,
skips lines carrying a `HISTORICAL` marker, strips Markdown emphasis, and flags any
`<nonzero> candidate|heuristic (run|runs|instance|instances)` triple.

The flagged sentence is not that kind of claim. It records a comparison probe:

> The unfiltered sample produces 32 heuristic instances in 31 contiguous groups;
> normal search returns none.

The tool's own vocabulary for what the scanner forms is **candidate**
(`bin/harness text search --no-readable`: *"Report every candidate the scanner forms,
without the readability floor (comparison only)"*). So the sentence is a truthful,
current description of scanner behaviour in contrast to the normal (filtered) result,
and the guard's pattern cannot tell it apart from a retired candidate-set claim.

Net effect: the repository's scoped gate is red for a documentation-versus-pattern
mismatch, and the failure blocks `just check` before it reaches the pre-existing
naming/binding debt at `justfile` line 67.

## What Changes

- Resolve the single flagged occurrence in `out/text-format/validation-summary.md` so
  the sentence is both truthful and unambiguous under the guard's pattern.
- Preserve the guard's actual coverage: after the change it must still flag a planted
  claim of the shape it was written for (`**N** candidate runs`, `N heuristic instances`
  as a *current-state candidate* claim).
- Do not weaken `RETIRED` or `HISTORICAL`, and do not add regression tests.

The exact remedy is decided by evidence in `design.md` — either the document adopts the
tool's own vocabulary, or the guard is narrowed to the candidate-set sense. Whichever is
chosen must leave both the claim truthful and the guard's real coverage intact.

## Capabilities

### New Capabilities
- `text-evidence-claim-guard`: the contract between `out/text-format/` evidence prose and
  the guard that rejects retired candidate figures — what the guard must catch, and what
  truthful prose it must not reject.

### Modified Capabilities
(none — no existing requirement covers the guard)

## Impact

- `out/text-format/validation-summary.md` (one sentence) and possibly
  `tools/rust/bof3-text/tests/text_payload_removals.rs` (the guard's pattern or markers).
- Verification: the guard's two tests pass, and a planted claim of the retired shape is
  still rejected.
- **Flagged for the ownership review in `docs/agents/harness.md`**: if the remedy touches
  the guard, this change alters harness test behaviour, which this repository's rules
  require to be explicit rather than incidental.
- Untouched: the text crate's search behaviour, `RETIRED`/`HISTORICAL` contents, the
  corpus, and every other `out/text-format` document.
