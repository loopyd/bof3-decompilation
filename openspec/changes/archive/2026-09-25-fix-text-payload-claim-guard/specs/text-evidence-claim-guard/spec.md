# Spec Delta

## Purpose

Defines the contract between the `out/text-format/` evidence documents and the guard in
`tools/rust/bof3-text/tests/text_payload_removals.rs` that rejects figures and
candidate claims the artifacts no longer support.

## ADDED Requirements

### Requirement: The guard rejects retired candidate claims

The guard SHALL reject a current-state claim that a nonzero number of candidate runs or
heuristic instances exists, in the documents it scans.

#### Scenario: A planted retired-shape claim is rejected

- **WHEN** a scanned document contains a current-state claim of the form
  `<nonzero> candidate runs` or `<nonzero> heuristic instances`
- **AND** the line carries no historical marker
- **THEN** the guard fails and names the document and line

#### Scenario: Retired figures stay rejected

- **WHEN** a scanned document quotes any figure listed in `RETIRED` on a line with no
  historical marker
- **THEN** the guard fails and names the figure

### Requirement: The guard accepts truthful scanner-behaviour prose

A sentence that describes what the scanner forms in an explicitly unfiltered or
comparison-only probe SHALL NOT be rejected, and SHALL be stated in the tool's own
vocabulary for that quantity.

#### Scenario: The comparison probe sentence passes

- **WHEN** the guard runs over `out/text-format/`
- **THEN** it reports no problems
- **AND** the sentence describing the unfiltered probe remains truthful and uses the
  tool's term for what the scanner forms

#### Scenario: The distinction is explicit in the document

- **WHEN** a reader meets that sentence
- **THEN** it is identifiable as an unfiltered or comparison-only probe, distinct from a
  current-state candidate claim

### Requirement: The guard's escape vocabulary is not weakened

`RETIRED` SHALL NOT shrink, and the set of historical markers SHALL NOT be broadened to
cover non-historical claims.

#### Scenario: Guard lists are intact

- **WHEN** the guard source is read
- **THEN** `RETIRED` still holds every figure it held before this change
- **AND** the guard's assertion that the retired list must not shrink still passes
- **AND** no new historical marker was added that would excuse a current-state claim
