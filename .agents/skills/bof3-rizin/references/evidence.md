# Evidence promotion

## Evidence levels

1. **Observed**: raw bytes, instruction operands, direct references, or a
   deterministic analyzer export.
2. **Correlated**: repeated stride, access width, call-site use, string, or spec
   association supports an interpretation.
3. **Reviewed**: target identity, address, size, and interpretation have been
   checked against disassembly or bytes.
4. **Promoted**: the reviewed fact is stored in its owning tracked artifact.

Analyzer guesses do not advance beyond observed without independent evidence.

## Promotion destinations

| Fact | Durable owner |
| --- | --- |
| Binary segment or boundary | `config/splat/` |
| Shared/authored symbol | `config/symbols/` or target `symbols.c` |
| Reproducible analysis rename/type link | `config/analysis/<target>.r2` |
| Recovered compiled type | owning `internal.h` or `include/bof3/` |
| Analysis-only cross-target type catalog | `config/analysis/bof3_objects.h` |
| Layout, ID, enum, or runtime contract | `docs/specs/` |
| Tool output, strings, xrefs, guesses | `out/analysis/` |

## Struct recovery checklist

- Identify the owning target and runtime address.
- Prove object stride from at least two references or a bounded table size.
- Record every observed access width and signedness.
- Separate byte layout from semantic interpretation.
- Use explicit unknown fields to preserve offsets.
- Validate `sizeof` and important offsets in C89-compatible declarations.
- Use the spec vocabulary. In this project the canonical enemy table type is
  `EnemyObject`; an upstream `MonsterObject` label is provenance, not an alias.
- Extract numeric IDs and hex values only when table bounds, index use, or a
  dispatch comparison provides evidence. Mark unresolved values `UNKNOWN:` or
  `INFERRED:` in authored docs as appropriate.

## Replay files

Tracked replay files must be deterministic, target-specific, and limited to
reviewed commands. The harness supplies engine, target identity, binary path,
load address, MIPS architecture, 32-bit width, and little-endian mode before
replay. Group commands in this order: function
names, data flags, type imports/links, comments. Never include UI state,
absolute workstation paths, generated names, or bulk analyzer guesses.
