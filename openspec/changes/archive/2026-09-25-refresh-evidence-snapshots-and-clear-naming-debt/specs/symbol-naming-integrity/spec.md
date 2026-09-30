# Spec Delta

## Purpose

Defines the repository's symbol-naming and binding integrity contract, together with the evidence
pipeline freshness the contract is proved through. This change restores that pipeline and records the
gate that still blocks the remaining debt; it applies no symbol rename itself.

## ADDED Requirements

### Requirement: The evidence pipeline is fresh enough to prove naming

The Rizin snapshots and the derived reverse index SHALL be fresh for every configured target, so the
reviewed naming transaction path and analysis queries run.

#### Scenario: The naming path opens

- **WHEN** `bin/harness naming init TARGET out/reviews/REPORT` runs for a target under review
- **THEN** it writes the report rather than failing on snapshot freshness

#### Scenario: The index rebuilds

- **WHEN** `just index` runs
- **THEN** it succeeds
- **AND** `bin/harness analysis query` answers instead of reporting a stale index

#### Scenario: A refreshed snapshot matches its target's current inputs

- **WHEN** a target's snapshot is refreshed because its config changed after the snapshot
- **THEN** the snapshot's recorded recipe equals the target's current recipe
- **AND** its recorded binary hash equals the current binary's

### Requirement: The naming debt's gate and disposition are recorded

The remaining raw-spelling debt SHALL be recorded with the gate that blocks its resolution and the
disposition of each row class, so the follow-on renaming change starts from measured facts rather
than assumptions.

#### Scenario: The blocking gate is recorded

- **WHEN** a prepared identity transaction is attempted for an audited row
- **THEN** the recorded gate is that `bin/harness naming prepare-transaction` refuses any row whose
  `rung_status` is not `proposed`
- **AND** the measured report state is recorded (1004 rows `blocked`, 0 rows `proposed`)

#### Scenario: The row classes are distinguished

- **WHEN** the debt rows are classified by symbol kind
- **THEN** function rows are recorded as carrying containment evidence, `analysis query --json owners`
  returning `payload_contained: 1` under `provenance: reviewed_range`
- **AND** data rows are recorded as carrying none — `describe` reporting `contained=false`,
  `present_in_binary=false`, `authority=[]`, with the true payload owner `exe/slus_004_22` carrying no
  symbol at the address

#### Scenario: The debt is not absorbed

- **WHEN** the change completes
- **THEN** `config/symbol-naming-baseline.json` is byte-identical to its pre-change content
- **AND** no entry was added by `bin/harness source symbols baseline --write`
- **AND** `bin/harness source symbols check` still reports the debt, so the follow-on change inherits
  it intact

#### Scenario: The gate's position is recorded

- **WHEN** the scoped gate is re-run
- **THEN** where it stops is recorded with its own evidence, whether or not it moves past
  `bin/harness source symbols check`

### Requirement: Naming decisions are reviewed and identity-preserving

Every symbol rename SHALL be prepared, applied and verified as one reviewed transaction, and no
touched target SHALL lose byte identity. This change applies no rename; the requirement fixes the
invariant the follow-on change must satisfy.

#### Scenario: A rename goes through the reviewed path

- **WHEN** a raw `func_*` or `D_*` spelling is replaced
- **THEN** it is prepared, applied and verified as one reviewed transaction
- **AND** a name with no supporting evidence is not applied
- **AND** the target's live byte comparison still reports exact identity afterwards

#### Scenario: Ownership evidence is per-kind

- **WHEN** a rename rests on an existing reviewed spelling carried by another target
- **THEN** a function row requires the owning target's payload to cover the address and its map to
  carry that name
- **AND** a data row requires same-target containment, because `owners` resolves functions only and
  returns `[]` for data in every target tested
- **AND** a spelling carried only by a different target remains a lead, requiring a shared map or
  recursive proof of identical address, content class and runtime role — a shared fixed-RAM
  promotion, not an audit-target identity

#### Scenario: A divergent address is resolved, not inherited

- **WHEN** one address carries more than one reviewed name across targets
- **THEN** the decision picks one on recorded evidence
- **AND** a target that keeps a different name records why
