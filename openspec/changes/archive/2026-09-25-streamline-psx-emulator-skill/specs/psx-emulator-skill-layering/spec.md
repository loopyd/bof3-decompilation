# Spec Delta

## Purpose

Defines the shape of the PSX emulator skill's entrypoint: a small layered front page over
an on-demand action catalog, with every existing ability preserved.

## ADDED Requirements

### Requirement: The entrypoint is small and layered

`SKILL.md` SHALL present a small number of named layers, each identifying the kind of
mission it covers and pointing to one owning reference, and SHALL NOT enumerate the full
per-action catalog.

#### Scenario: Layers are named and few

- **WHEN** the entrypoint is read
- **THEN** it states a small number of named layers with the missions each covers
- **AND** each layer points to exactly one owning reference

#### Scenario: The catalog is not on the front page

- **WHEN** the entrypoint is read
- **THEN** the full per-action catalog is not enumerated there

### Requirement: Every ability is preserved

The regrouped skill SHALL keep every action, reference, script and test that existed
before the change.

#### Scenario: The catalog lists every action

- **WHEN** the on-demand catalog is read
- **THEN** it lists every action the previous entrypoint listed, each with its owning
  reference
- **AND** a script still exists for each action

#### Scenario: Nothing is deleted

- **WHEN** the skill's files are compared with the baseline recorded before the change
- **THEN** no reference, script or test has been removed

### Requirement: A mission loads one layer plus its action

A mission SHALL be able to reach its action's reference through the entrypoint and a
single layer, without loading unrelated layers.

#### Scenario: Only the needed layer is read

- **WHEN** a mission selects an action
- **THEN** the entrypoint and the owning layer reference identify that action's reference
- **AND** no unrelated layer must be read to begin the mission

### Requirement: The entrypoint stays broad in coverage

The entrypoint SHALL keep every previously available capability family reachable, so
that reducing its size does not narrow its scope.

#### Scenario: Breadth is preserved

- **WHEN** the layers are read
- **THEN** every capability family the previous catalog listed is reachable through some
  layer

### Requirement: Owning documents agree with the new shape

The dispatch spec and the documentation routes SHALL point at the layered entrypoint and
its catalog rather than the retired flat listing.

#### Scenario: Dispatch and routes still resolve

- **WHEN** `.pi/agents/psx-emulator.md` and the affected docs routes are read
- **THEN** they point at the layered entrypoint and its catalog
- **AND** no link from them is broken
