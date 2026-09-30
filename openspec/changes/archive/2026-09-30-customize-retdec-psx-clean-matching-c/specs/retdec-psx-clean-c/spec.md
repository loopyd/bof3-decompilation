# PSX clean C output

## Purpose

Define ordinary PSX C output that preserves evidenced machine behavior and validated matching results without hiding unsupported execution paths.

## ADDED Requirements

### Requirement: Ordinary source output

The customized decompiler SHALL emit ordinary, readable C89 for supported PSX functions. Successful output MUST compile unchanged with the function's established repository compiler profile and supplied declarations. It MUST NOT require register pins, artificial clobbers, inline assembly substitutes, instruction-emulation wrappers, or manual generated-body corrections to claim clean-C success.

#### Scenario: Supported function output

- **WHEN** an existing supported evaluation function is decompiled with its pinned inputs and declarations
- **THEN** the unedited output compiles with its recorded profile
- **AND** the acceptance report records source review against the prohibited aids
- **AND** unresolved dependencies prevent a successful ordinary-C execution claim

### Requirement: Preserve evidenced machine behavior

The output SHALL preserve operation widths, signedness, wraparound, memory access effects and ordering, and delayed-load behavior within the declared supported domain. Source recovery MUST retain justified types and qualifications without narrowing the machine entry contract or inventing object boundaries. Missing evidence MUST leave the affected recovery unapplied or explicitly unsupported.

#### Scenario: Existing arithmetic and memory cases

- **WHEN** regenerated output is run through the retained arithmetic, raw-register, and little-endian memory evaluations
- **THEN** its results agree with the original instructions for the covered inputs
- **AND** the report identifies the exact checked domain and all unsupported cases

#### Scenario: Qualified counter recovery

- **WHEN** the retained counter is regenerated with its evidenced declarations
- **THEN** the output preserves byte wraparound, the value tested by the comparison, and the qualified memory accesses
- **AND** any claimed improvement includes original-byte comparison and bounded behavior evidence
- **AND** a numerically similar address alone does not justify applying the same qualification to other accesses

### Requirement: Proven exception-path simplification

The decompiler SHALL eliminate an exceptional path only when evidence proves it unreachable under the declared input contract. It MUST NOT treat failure to observe a trap as such proof. Reachable or unproven exceptional paths MUST remain explicit and MUST NOT be accepted as ordinary-C execution using placeholder handlers or silently restricted inputs.

#### Scenario: Unreachable compiler guard

- **WHEN** analysis of an existing division-guard case proves its exceptional conditions impossible for every admitted input
- **THEN** the generated ordinary C can omit that path
- **AND** the report retains the proof, assumptions, and original instruction identity

#### Scenario: Unresolved compiler guard

- **WHEN** reachability analysis is inconclusive or establishes a reachable exception
- **THEN** the function remains explicitly blocked for ordinary-C execution
- **AND** compilation or linking alone does not change that disposition

### Requirement: Per-function non-regression

The delivered customization SHALL preserve every validated byte-exact match and bounded semantic result in the frozen acceptance corpus under unchanged inputs, supplied metadata, and compiler profiles. Comparisons MUST cover complete function extents and required placement. Semantically validated non-exact baselines MUST NOT lose their recorded instruction agreement through candidate replacement. Aggregate improvements MUST NOT offset an individual regression.

#### Scenario: Proposed recovery regresses a validated function

- **WHEN** a new recovery improves one function but loses another function's validated result
- **THEN** that replacement is rejected or the validated baseline is retained through an evidence-backed general rule
- **AND** the delivery cannot pass with the individual regression hidden by totals or a changed corpus

#### Scenario: Historical score lacks semantic validation

- **WHEN** a historical variant has a higher similarity score but lacks semantic support or has a known semantic defect
- **THEN** it is reported separately from validated matching baselines
- **AND** its score does not justify selection or removal of a semantic correction

### Requirement: Distinct evidence levels

Acceptance results SHALL distinguish generation, compilation, linkage, instruction similarity, complete byte identity, and bounded runtime agreement per function. Signature-only and metadata-assisted results MUST remain separate. Existing excluded references and blocked functions MUST remain visible unless new evidence justifies their changed disposition.

#### Scenario: Broad corpus rebuild

- **WHEN** the retained broad corpus and six-function sample are regenerated
- **THEN** each result identifies its inputs, metadata, compiler profile, and evidence level
- **AND** functions that compile without function-specific runtime evidence are not reported as newly runtime-validated
