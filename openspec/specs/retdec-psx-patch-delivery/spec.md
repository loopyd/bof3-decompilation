# retdec-psx-patch-delivery Specification

## Purpose

Provide one complete, reproducible customization patch for the RetDec plugin and its required sources, with evidence tied to the delivered artifact.

## Requirements

### Requirement: Single complete customization patch

Delivery SHALL contain exactly one cumulative customization patch covering the plugin changes and every required dependency source addition or modification. The delivery MUST identify its source revisions, application layout, prerequisites, and patch checksum. It MUST NOT depend on an earlier repair patch, an untracked local source file, an existing build directory, or an undocumented manual edit.

#### Scenario: Reconstruct from clean inputs

- **WHEN** a user follows the delivered instructions using clean copies of the identified sources and existing prerequisites
- **THEN** the single patch supplies every required customization
- **AND** the resulting plugin builds without consulting the original evaluation workspace

### Requirement: Reproduce accepted output

The delivered patch SHALL reproduce the accepted generated source and comparison outcomes from a clean build. Verification MUST identify the plugin built from that patch and the exact corpus inputs used. Stale binaries MUST NOT count as delivery evidence.

#### Scenario: Clean-build acceptance replay

- **WHEN** the plugin built from the delivered patch runs the frozen acceptance corpus
- **THEN** the report binds the patch, build, emitted-source identities, and results
- **AND** validated byte-exact and semantic results remain preserved
- **AND** any output difference is resolved before delivery is declared complete

### Requirement: Reversible isolated application

Patch verification SHALL reject incompatible base sources and SHALL demonstrate forward and reverse application on the identified clean layout. Verification MUST preserve installed tools, production sources, private inputs, and unrelated dirty work.

#### Scenario: Wrong base source

- **WHEN** a source revision or required base file differs from the documented baseline
- **THEN** preflight fails before applying the customization
- **AND** the failure identifies the incompatible input

#### Scenario: Reverse application

- **WHEN** the patch is reversed on the disposable verification layout after forward application
- **THEN** source identities match the recorded clean baseline, including absence of newly added files

### Requirement: Honest release report

Delivery SHALL include an acceptance report that distinguishes completed repairs, preserved matches, unresolved cases, and validation limitations. The report MUST contain no private game bytes and MUST NOT claim production integration or universal matching from bounded evaluation results.

#### Scenario: An exceptional function remains blocked

- **WHEN** ordinary-C exception reachability remains unproven at delivery
- **THEN** the report names the blocked function and missing proof
- **AND** a reproducible patch does not imply that the function is supported
