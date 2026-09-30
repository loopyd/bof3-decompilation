# Spec Delta

## Purpose

Keeps repository documentation navigable: every reference under the swept
documentation roots resolves to a target that exists, and every fragment names a
heading that exists.

## ADDED Requirements

### Requirement: References resolve to existing targets

Every Markdown reference under the roots covered by `bin/harness docs refs` SHALL
resolve to an existing file, and SHALL NOT resolve outside the repository root.

#### Scenario: The reference sweep is clean

- **WHEN** `bin/harness docs refs docs .pi/skills --broken-only` runs
- **THEN** it reports no broken references
- **AND** it exits zero

#### Scenario: A reference does not escape the repository root

- **WHEN** a reference in a skill reference file is resolved relative to that
  file's directory
- **THEN** the resolved path stays inside the repository root
- **AND** the referenced file exists

### Requirement: Fragments name existing headings

A reference carrying a fragment SHALL name a heading that exists in its target
document.

#### Scenario: Request-router fragments resolve

- **WHEN** `docs/INDEX.md` routes a request to a section of another document
- **THEN** the fragment matches a heading present in that document

### Requirement: Retargeted references name the current owner

Where the reorganisation of `tools/rust/bof3-audio` moved the module or test file
a reference named, the reference SHALL be updated to the current owner of the
described behaviour.

#### Scenario: Crate references follow the reorganisation

- **WHEN** a reference names a path under `tools/rust/bof3-audio/src` or
  `tools/rust/bof3-audio/tests` that the reorganisation moved
- **THEN** the reference names the current module or test file for that behaviour
- **AND** the surrounding claim remains true of the newly named owner

#### Scenario: A split claim names each owner

- **WHEN** the behaviour a single reference described is now spread across more
  than one module
- **THEN** the reference targets the module or directory that owns each claim
- **AND** no reference points at a path that no longer exists
