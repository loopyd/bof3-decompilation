# BOF3 evidence and promotion

## Evidence levels

1. **Observed**: raw bytes, decoded instruction operands, direct xrefs, exact
   strings, or deterministic analyzer export.
2. **Correlated**: repeated stride/access width/call shape, SDK declaration,
   string association, or cross-target comparison supports an interpretation.
3. **Reviewed**: exact target, runtime address, size/boundary, and interpretation
   were checked against disassembly or bytes.
4. **Promoted**: the reviewed fact is stored in its durable owning artifact and
   can be replayed or compiled.

Analyzer/decompiler guesses do not advance beyond observed without independent
evidence.

## Promotion destinations in this repository

| Fact | Durable owner |
| --- | --- |
| Binary segment, load address, boundary | `config/splat/` or target manifest |
| Shared/authored symbol | `config/symbols/` or target `symbols.c` |
| Reviewed analyzer rename/comment/type placement | `config/analysis/<target-path>/reviewed.r2` |
| Analysis-only cross-target type catalog | `config/analysis/shared/bof3_objects.h` |
| Recovered compiled type | owning `internal.h` or `include/bof3/` |
| Layout, ID, enum, runtime contract | `docs/specs/` |
| Projects, strings, xrefs, pseudocode, guesses | `out/analysis/` |

## Function and data naming checklist

- Verify target identity and runtime load address.
- Verify the address is code or data and its exact boundary/size.
- Keep `func_XXXXXXXX`/`DAT_XXXXXXXX` traceability until meaning is reviewed.
- Use callsites, xrefs, access widths, strings, state transitions, and duplicate
  evidence together; do not rename from one decompiler label.
- Keep the binding target-local unless independent cross-target evidence proves
  the same semantic object.
- Preserve an `INFERRED:` comment and verification path for strong but unproven
  interpretations instead of promoting the name.

## Struct and type recovery checklist

- Identify the owning target and runtime address.
- Prove stride from multiple references or a bounded table size.
- Record observed field offsets, access widths, signedness, alignment, and array
  bounds before semantic labels.
- Use explicit unknown fields to preserve layout.
- Validate `sizeof` and important offsets in C89-compatible declarations.
- Check pointer width and MIPS argument/return use.
- Treat decompiler unions, casts, and inferred prototypes as provisional.
- Apply a type only after imported layout and runtime placement are separately
  reviewed.

Keep three claims distinct: an observed offset/width/signedness, its semantic
field interpretation, and the current C candidate's match status. Reviewed raw
loads/stores can establish a layout fact while the lifted C remains non-exact.
Report both facts; an exact match strengthens the source reconstruction but is
not required to record an independently reviewed access-width observation.

## PsyQ signature correlation

Track four independent facts:

1. official SDK header declaration and SDK version;
2. library/archive member provenance when known;
3. observed call shape and assembly behavior;
4. target-local runtime address/binding.

Promote an official name only when the prototype, call shape, and assembly
agree. Do not assume a library address repeats across executables, overlays, SDK
versions, or duplicated payloads. Do not lift or replace verified library code.

For structs, enums, constants, and macros, distinguish source-level SDK values
from emitted runtime data. Confirm packing/layout and compiler-version behavior.

## Cross-target duplicates

Exact bytes prove byte identity for a bounded range, not automatically shared
runtime addresses, relocatability, ownership, or semantic state. Relocation-
masked similarity is only a candidate. Keep independent target projects and
bindings until relocations, call targets, data references, and behavior are
reviewed.

## Replay review

Group tracked commands in deterministic order: functions/names, data flags,
type imports/placements, comments. Exclude UI state, absolute machine paths,
bulk analyzer guesses, decompiler text, and generated output. Rebuild a clean
project and compare deterministic exports before calling replay reproducible.

## Bounded emulation evidence

Use ESIL only to test a concrete computed-reference or indirect-flow hypothesis
over a bounded instruction range with explicit initial register/memory state.
Record those assumptions and compare the result with raw instructions and
xrefs. ESIL output is analyzer evidence, not proof of runtime behavior, target
identity, or a function boundary.

Prefer staged analysis and reviewed calling conventions before propagating
types. Broad analysis, inferred prototypes, and decompiler propagation amplify
wrong boundaries quickly.

References: [Rizin analysis](https://book.rizin.re/src/analysis/index.html),
[radare2 code analysis](https://book.rada.re/analysis/code_analysis.html), and
[radare2 ESIL](https://book.rada.re/disassembling/esil.html).
