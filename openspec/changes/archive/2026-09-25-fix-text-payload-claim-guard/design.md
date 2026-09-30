# Design

## Context

See `proposal.md` — Why. Facts established read-only:

- The guard scans every `out/text-format/*.md`; it skips a line when it contains any of
  the nine `HISTORICAL` markers, strips `*`, `` ` `` and `_`, then flags any
  `<count> <candidate*|heuristic*> <run|runs|instance|instances>` triple whose count is
  nonzero. `RETIRED` holds 14 figures, and the guard asserts the list never shrinks.
- It fired on exactly one line of one document:
  `out/text-format/validation-summary.md:20`.
- `bin/harness text search --no-readable` documents the quantity as *"every candidate the
  scanner forms, without the readability floor (comparison only)"* — the tool's word is
  **candidate**, not *instance*.
- A second sentence in the same document ("missed the 32 instances found by a fresh
  build") does **not** match the pattern, so the flagged text is a single occurrence.

## Goals / Non-Goals

**Goals:**

- Make the scoped gate green at this guard without disabling the guard.
- Keep every claim in the evidence document truthful.
- Keep the guard's real coverage: a current-state candidate claim must still fail.

**Non-Goals:**

- No change to search behaviour, the corpus, or any other `out/text-format` document.
- No test additions; at most a narrow refinement of the existing guard.
- Do not shrink `RETIRED` or widen `HISTORICAL`.

## Decisions

### Decision 1: decide the remedy by which reading is true, not by which is easier

Two remedies are possible and the choice is an evidence question:

- **(a) Document-side** — restate the probe in the tool's vocabulary ("32 heuristic
  candidates in 31 contiguous groups"), which is truthful and no longer collides with the
  pattern, and label it as the comparison-only probe it is.
- **(b) Guard-side** — narrow the pattern so an explicitly unfiltered/comparison-only
  claim is excluded, leaving the candidate-set sense still covered.

*(a) is preferred when the sentence is simply using a non-standard word for a quantity the
tool calls candidates — it fixes the wording rather than the detector, and changes no
harness behaviour. (b) is required if reproducing the probe shows the sentence is not a
scanner-candidate claim at all, or if other legitimate prose also collides.*

### Decision 2: the flag must stay honest

Rewriting "instances" to "candidates" is only acceptable if the number still describes
what the scanner forms. If the probe cannot be reproduced, the sentence is marked as the
recorded result of the named probe rather than silently restated as current, so the guard's
premise ("current-state claims must be zero") is honoured rather than evaded by vocabulary.

### Decision 3: prove the guard still works by planting a claim

Verification plants a sentence of the retired shape (`**N** candidate runs`) in a scanned
document, confirms the guard fails on it, then removes it. *Rejected:* trusting that an
unchanged pattern still works — the whole point of this change is that the pattern's
behaviour needed judgement. Planting is a verification step, not an added regression test.

### Decision 4: treat a guard change as harness behaviour

If remedy (b) is chosen, the change modifies harness test behaviour, which this
repository's rules require to be explicit and reviewed rather than incidental, so the
proposal flags it for the ownership review in `docs/agents/harness.md`.

## Risks / Trade-offs

- [Evading the guard by vocabulary rather than truth] → Decision 2 requires the restated
  number to describe the quantity the tool calls candidates, or to be labelled as a
  recorded probe.
- [Quietly weakening a guard that exists because a claim leaked once] → Decision 3 plants
  the exact shape the guard was written for and requires it to fail.
- [Editing `out/` state that the repository treats as disposable] → only one sentence in
  one document is touched, and the document is a hand-authored evidence record that a test
  deliberately guards, not generated build output.
