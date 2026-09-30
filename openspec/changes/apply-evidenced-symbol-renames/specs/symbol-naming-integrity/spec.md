# Spec Delta

## REMOVED Requirements

### Requirement: The naming debt's gate and disposition are recorded

**Reason**: Superseded twice over. The previous change recorded the *audit* gate and the row
dispositions; this change has now established that the binding limitation is not that gate but a
**missing harness capability**, so a requirement about recording gates and dispositions no longer
describes the contract that matters.

**Migration**: The added requirement "The naming path's capability gap is recorded" carries the
replacement contract, with the four gates, the live observations and the blocked backlog as its
scenarios. The evidence-freshness requirement and the reviewed-path invariant are unaffected. The
resolution and binding-composition requirements this change previously proposed are **withdrawn**:
they require a harness capability that does not exist, and they belong to the change that can satisfy
them rather than to a change that cannot run them.

## ADDED Requirements

### Requirement: The naming path's capability gap is recorded

The repository SHALL record why the reviewed naming transaction path cannot resolve the raw-spelling
debt, and what backlog that leaves, so that the limitation is neither rediscovered nor mistaken for an
evidence problem.

#### Scenario: The blocking gates are recorded with their code locations

- **WHEN** the capability gap is recorded
- **THEN** it names the gate that rejects every `proposed`/`exhausted` row lacking a runner-recomputed
  capability, the gate that returns no capability for any non-data row, the terminal-capability gate,
  and the absent producer for the `selected_call` and `owner_body` facts
- **AND** each is cited by file and line in `tools/python/harness/naming/`

#### Scenario: The repository's own tests are cited as intent

- **WHEN** the gap is recorded
- **THEN** the naming-conclusion tests that assert the rejection are named, so the behaviour is
  recorded as intended rather than as a defect

#### Scenario: The backlog the gap blocks is recorded

- **WHEN** the change completes
- **THEN** the recorded backlog is the measured one — 75 findings: 61 rows (40 function, 21 data) over
  9 symbols and 59 targets, plus 14 `binding/map drift`s
- **AND** `bin/harness source symbols check` still reports those findings, recorded as-is
- **AND** `config/symbol-naming-baseline.json` is byte-identical to its pre-change content, with no
  entry added by `bin/harness source symbols baseline --write`

#### Scenario: The handoff names its own prerequisite

- **WHEN** the unblocking work is handed off
- **THEN** it is recorded as a harness behaviour change requiring the ownership review in
  `docs/agents/harness.md`
- **AND** it is not presented as something a naming change can absorb as a side effect
