# Spec Delta

## Purpose

Defines the shape and integrity of the repository's ladder guidance: what a ladder must
state, what survives simplification, and how one ladder avoids duplicating another.

## ADDED Requirements

### Requirement: A ladder states one rung per step with its evidence gate

Each ladder SHALL present its rungs as an ordered sequence in which every rung names the
action it applies and the evidence that accepts or rejects the result.

#### Scenario: A rung is actionable on its own

- **WHEN** a ladder rung is read in isolation
- **THEN** it names the change to make
- **AND** it names the evidence that decides whether the rung succeeds

#### Scenario: Rungs stay ordered and terminate

- **WHEN** a ladder is applied
- **THEN** its rungs are applied in the stated order
- **AND** the first rung that resolves the residual ends the ladder

### Requirement: Simplification preserves every gate, ban and obligation

Compacting ladder guidance SHALL NOT remove or weaken a rung, an evidence gate, a ban, a
threshold, an approval requirement or an unresolved obligation.

#### Scenario: Prohibitions survive

- **WHEN** the simplified guidance is read
- **THEN** every prohibition it previously stated is still stated, including the ban on
  register pins, direct asm register bindings, `CLOBBER_*`, artificial barriers and
  evidenced empty-asm matching barriers

#### Scenario: Approval and exhaustion gates survive

- **WHEN** the simplified guidance is read
- **THEN** `INCLUDE_ASM` still requires separate explicit approval
- **AND** the exhaustion gate and its `ladder_exhausted` reporting conditions are still
  stated
- **AND** opt-in expensive rungs still require explicit authorization for that selector

#### Scenario: Acceptance rules are unchanged

- **WHEN** any affected document or skill is compared with its pre-change text
- **THEN** no acceptance rule, threshold or required evidence has been relaxed

### Requirement: A ladder has one home and references the others

Where a lever table or rule already has an owner, other guidance SHALL link to that owner
instead of restating it.

#### Scenario: Lever tables are referenced, not duplicated

- **WHEN** a skill ladder needs lever guidance owned by `docs/agents/matching-playbook.md`
  or `references/OBSERVATIONS.md`
- **THEN** it links to that owner rather than restating the table

### Requirement: Harness capabilities are referenced, not restated

Skill guidance SHALL cite the harness command or owning document it depends on, and SHALL
NOT restate that command's full CLI surface.

#### Scenario: A skill cites rather than restates

- **WHEN** a skill needs a harness capability
- **THEN** it names the command
- **AND** it links to the owning document for that command's options and gates

### Requirement: The accessible size of the guidance is reduced

The simplified and compacted guidance SHALL be measurably smaller without losing any
statement required above.

#### Scenario: Reduction is measured and preservation is verified

- **WHEN** the change is verified
- **THEN** the ladder-bearing and compacted files are smaller in line count than before
  the change
- **AND** the gate vocabulary enumerated in this spec is still present in the file that
  previously stated it
