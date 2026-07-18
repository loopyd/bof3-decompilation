---
name: workflow-review
description: Run an explicitly invoked, two-perspective adversarial review of BOF3 workflows, tools, skills, documentation, and architecture. Use only when the user invokes `$workflow-review`; do not use for ordinary one-function lift acceptance.
---

# Workflow Review

Review the requested scope against user intent and repository evidence. Do not
expand the task merely because an improvement is possible.

## Run two reviewers

Spawn exactly two bounded, read-only reviewers in parallel:

1. **Correctness and evidence**: inspect identity, binary ownership, bytes,
   boundaries, maps, Splat, declarations, Rizin freshness, output/write
   contracts, destructive behavior, compatibility, and skipped verification.
2. **KISS and handoff**: inspect YAGNI, files and plumbing, duplicated or stale
   docs, context budgets, predictable user-editable steps, small-model
   ambiguity, naming, locality, and maintenance cost.

Give both reviewers the raw scope or diff and success contract, not an expected
verdict. They must not edit. Require this shape:

```text
Verdict:
Blocking:
Non-blocking:
Questions:
Checks:
```

## Reconcile and fix

1. Resolve disagreements using original bytes, explicit contracts, and user
   intent rather than reviewer votes.
2. Fix only in-scope blockers automatically.
3. Rerun affected evidence checks and both reviews after material fixes.
4. Present non-blocking or scope-expanding changes as suggestions for the user
   to choose; do not implement them silently.

Use risk-based reverse-engineering evidence, not test-count gates. Never add
game-behavior tests. For tooling changes, retain or add only the least test that
protects a real parsing, isolation, freshness, output, or write contract.

## Deliver

Default to a compact verdict with blockers, evidence, checks, skipped checks,
and next user decision. Provide extensive evidence only when requested or
needed to explain disagreement. Never commit unless the current user
explicitly requests it.
