# Spec Delta

## Purpose

Defines how this repository configures OpenSpec so that every artifact it
generates is authored against BOF3's hard contract and per-artifact conventions
instead of the tool's generic defaults.

## ADDED Requirements

### Requirement: Repository context reaches every artifact instruction

`openspec/config.yaml` SHALL declare a non-empty `context` value, and every
artifact instruction this repository emits SHALL carry that value in its
`context` field.

#### Scenario: Context is present in an artifact instruction

- **WHEN** `openspec instructions proposal --change <name> --json` runs for any
  change in this repository
- **THEN** the response contains a non-empty `context` field
- **AND** that context names the repository's owning authority documents
  (`AGENTS.md` and `docs/INDEX.md`) rather than restating their contents

#### Scenario: Context stays within the configured limit

- **WHEN** OpenSpec reads the repository context
- **THEN** the value is below the 50 KB context limit
- **AND** no "Context too large" warning is printed

### Requirement: Per-artifact authoring rules

The configuration SHALL declare a non-empty `rules` entry for each artifact id of
the active `spec-driven` schema: `proposal`, `specs`, `design` and `tasks`.

#### Scenario: Rules accompany the matching artifact

- **WHEN** `openspec instructions <artifact> --change <name> --json` runs for
  `proposal`, `specs`, `design` and `tasks`
- **THEN** each response contains a non-empty `rules` array
- **AND** no unknown-artifact-id warning is printed, because every key matches an
  artifact of an installed schema

#### Scenario: Task rules carry the repository's execution constraints

- **WHEN** the `tasks` instruction is generated
- **THEN** its rules require every task to state how completion is verified
- **AND** they forbid adding regression tests or new dependencies unless the
  operator explicitly requested them

### Requirement: Apply and archive guidance

The configuration SHALL declare guidance for the `apply` and `archive`
operations.

#### Scenario: Operation guidance accompanies its operation

- **WHEN** `openspec instructions apply --change <name> --json` and `openspec
  instructions archive --change <name> --json` run for a change whose tasks exist
- **THEN** each response carries the configured guidance for that operation in
  `operationGuidance`

### Requirement: Configuration parses without warnings

The configuration SHALL parse against the OpenSpec project-config schema without
field warnings or unknown-artifact warnings.

#### Scenario: Doctor reports a healthy root

- **WHEN** `openspec doctor` runs
- **THEN** the resolved root is reported ok
- **AND** no configuration warning is emitted for the file
