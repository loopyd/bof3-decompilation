# target-map-normalization Specification

## Purpose
Defines the contract that every target map is in the canonical form its own formatter produces, so
map formatting is never a source of drift.

## Requirements

### Requirement: Target maps are normalized

Every target map SHALL be in the canonical form the map formatter produces.

#### Scenario: No unnormalized map remains

- **WHEN** `bin/harness source symbols check` runs
- **THEN** it reports no `unnormalized map`

#### Scenario: The formatter is idempotent over its targets

- **WHEN** the formatter is re-run over the targets it just formatted
- **THEN** it produces no further change

### Requirement: Normalization clears formatting drift only

Normalizing a map SHALL NOT alter symbol spelling, addresses, ownership or the pinned baseline.

#### Scenario: The baseline is unchanged

- **WHEN** the change completes
- **THEN** `config/symbol-naming-baseline.json` is byte-identical to its pre-change content

#### Scenario: No naming debt is created or cleared

- **WHEN** `bin/harness source symbols check` runs before and after the change
- **THEN** the naming-debt rows are unchanged in kind and count
- **AND** only the `unnormalized map` rows are cleared
