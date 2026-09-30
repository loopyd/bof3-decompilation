# Spec Delta

## Purpose

Defines the harness's typed naming-evidence facts and the capability gate that admits a `proposed` or
`exhausted` naming conclusion, so that every rung a row requires has a producer and admission rests on
runner-produced evidence rather than on author assertion.

## ADDED Requirements

### Requirement: Every required rung has a fact producer

For every rung the audit contract declares required for a row's kind and locality, the harness SHALL
be able to produce positive typed facts, and a rung with no producer SHALL be reported as an explicit
capability gap rather than left as permanently unclosable work.

#### Scenario: The imported-function rungs can close

- **WHEN** a row is an imported function and requires `selected_call` and `owner_body`
- **THEN** the harness produces positive facts for both, each backed by a runner receipt recording
  the executed command and its typed observations
- **AND** `selected_call` records the instruction-level callsite including its delay slot, argument
  registers, guards, result and transition
- **AND** `owner_body` records the owning function's body effects, callees, globals, tables and
  consumers

#### Scenario: A missing producer is reported, not hidden

- **WHEN** a required rung has no producer
- **THEN** the absence is reported as a capability gap naming that rung
- **AND** the row is left explicitly blocked rather than presented as closable work

### Requirement: The capability gate admits a proposal without weakening authenticity

The capability gate SHALL admit a `proposed` or `exhausted` conclusion for any row whose required
rungs are closed by runner-produced facts bound to the current report, and SHALL continue to reject
any row that lacks such a capability.

#### Scenario: A proposal with a current capability is admitted

- **WHEN** a row's required rungs are closed by runner-produced facts and the conclusion binds the
  current report digest
- **THEN** the conclusion is admitted, for function and data rows alike

#### Scenario: A proposal without a capability is still refused

- **WHEN** a `proposed` or `exhausted` conclusion has no runner-recomputed capability, or its binding
  to the current report is stale
- **THEN** it is refused, and the refusal names the missing or stale capability rather than the
  author's evidence quality

#### Scenario: Authored evidence cannot substitute for runner facts

- **WHEN** an authored conclusion supplies observations, receipts or digests that the runner did not
  produce for that row
- **THEN** it is refused, because recovery artifacts are not an authenticity boundary

### Requirement: The gate's allowance is derived, not enumerated

The rows for which a capability can exist SHALL be derived from the row's own kind, locality and
closed rungs, so that a new target or symbol does not need a hand-maintained allowlist entry before it
can be admitted.

#### Scenario: A row outside the historical allowlist can be admitted

- **WHEN** a row's required rungs are closed by runner-produced facts, in a target that has no
  hand-maintained registry entry
- **THEN** the gate admits it on the same terms as an allowlisted row
