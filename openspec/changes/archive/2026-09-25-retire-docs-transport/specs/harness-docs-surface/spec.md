# Spec Delta

## Purpose

Defines the harness's Markdown surface: which Markdown capabilities exist as
commands, which remain internal library code behind the surviving gates, and which
obligations the documentation skill carries on its own once the transport is gone.

## ADDED Requirements

### Requirement: The Markdown transport is not a harness command

The harness SHALL NOT expose a Markdown read/search/edit transport command, and the
capabilities that transport provided SHALL be performed with native file tools.

#### Scenario: The command is gone

- **WHEN** `bin/harness docs refs docs/agents` is run
- **THEN** the harness reports an unknown domain or command
- **AND** `bin/harness --help` does not list a `docs` domain

#### Scenario: No transport module remains reachable

- **WHEN** the harness imports are inspected
- **THEN** `harness.docs.cli`, `harness.docs.documents`, `harness.docs.references`,
  `harness.docs.markdown` and `harness.docs.anchors` do not exist
- **AND** `harness.registry` declares no domain that resolves to them

### Requirement: The documentation skill operates without a transport

The `$bof3-docs` skill SHALL perform its work with native file tools, and SHALL
retain every obligation the transport previously supported.

#### Scenario: The skill names no retired command

- **WHEN** the skill and its references are read
- **THEN** no instruction invokes `bin/harness docs` or a wrapper that forwards to it

#### Scenario: Obligations survive the retirement

- **WHEN** the skill is applied to a scoped Markdown change
- **THEN** it still requires checking the owning implementation, configuration,
  specification or policy before editing
- **AND** it still requires preserving unrelated wording, resolving affected links
  and index entries, running the applicable existing checks and reporting gaps

### Requirement: Surviving gate capabilities are unchanged

The retirement SHALL NOT change the behaviour of the capabilities that remain.

#### Scenario: The drift gate still runs

- **WHEN** `bin/harness source docs` runs
- **THEN** it reports its findings as before
- **AND** it exits zero when there is no drift

#### Scenario: Other library consumers keep working

- **WHEN** `bin/harness agent context cleanup docs PATHS...` runs
- **THEN** its path validation still resolves as before

### Requirement: Documented owners match the actual surface

Documents and skills SHALL NOT advertise a harness command that no longer exists.

#### Scenario: No stale transport instructions remain

- **WHEN** `docs/` and `.pi/skills/` are searched for the retired command
- **THEN** no document or skill instructs a reader to run it
- **AND** the document contract still describes reference inspection, keeping the
  heading that `docs/INDEX.md` links to
