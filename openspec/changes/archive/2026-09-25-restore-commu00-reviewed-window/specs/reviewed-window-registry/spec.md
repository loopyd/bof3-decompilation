# Spec Delta

## Purpose

Defines the contract between a target record's reviewed `textbin` ranges and the
classified-window registry: which windows must appear, and when a freshly built registry
counts as matching the retained one.

## ADDED Requirements

### Requirement: Reviewed ranges reach the registry

Every reviewed `textbin` range a target record declares SHALL appear in the classified-window
registry as a window naming that record, its archive and its entry, and the build SHALL fail
rather than silently produce a smaller registry when a range the boundary record names is
absent.

#### Scenario: The named reviewed ranges are present

- **WHEN** `bin/harness text windows` runs against `config/targets/emi`
- **THEN** the registry contains a `reviewed_textbin` window for `ETC/COMMU00.EMI` entry 0,
  for `SCENARIO/SCENA00.EMI` entry 0 and for `WORLD00/AREA026.EMI` entry 13
- **AND** the command exits zero

#### Scenario: A lost range fails the build

- **WHEN** any reviewed range the boundary record names yields no window
- **THEN** the build fails naming the target root and the missing `archive#entry`
- **AND** no registry is written in place of the retained one

### Requirement: A window agrees with its record

Each reviewed window SHALL state the range its record declares: the same start offset, the
length bounded by the record's next entry, and — when the record names a `T_` symbol — a name
equal to `load_address + offset`.

#### Scenario: The window matches the record

- **WHEN** the COMMU00 window is read from the registry
- **THEN** its `start` is the record's textbin offset and its `length` is the distance to the
  next record entry
- **AND** its name equals the record's `load_address + start`

### Requirement: A fresh build matches the retained registry

A freshly built registry SHALL be byte-identical to the retained artifact, so drift between
the records and the retained registry fails rather than passing silently.

#### Scenario: Fresh build equals retained

- **WHEN** `cargo test --test text_window_registry` runs
- **THEN** the freshly built registry is byte-identical to the retained one
- **AND** the test passes with the retained artifact unchanged by the test run itself

### Requirement: The guard's expected set is not weakened

The set of reviewed ranges the build requires SHALL NOT be shortened, and the failure on an
absent one SHALL NOT be downgraded to a warning.

#### Scenario: The expected set is intact

- **WHEN** the window builder's source is read
- **THEN** it still requires all three named `archive#entry` pairs
- **AND** a missing one still fails the build
