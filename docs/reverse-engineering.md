# Reverse engineering

> BOF3 is a set of independently loaded binaries and resources, not one linked C program.

## Binary model

| Item | Identity | Durable location |
| --- | --- | --- |
| Main executable | `SLUS_004.22` load image | `out/binaries/exe/slus_004_22.bin` |
| Logo executable | `LOGO.EXE` load image | `out/binaries/exe/logo.bin` |
| EMI archive | shipped archive path | `out/extracted/` |
| EMI entry | archive path + slot | `out/catalog/emi.json` |
| Confirmed code module | entry plus runtime address | `config/splat/emi/`, `src/emi/` |

Splat operates on normalized executable images or extracted EMI entries. It
never operates on an entire EMI container. Generated assembly belongs below
`out/splat/`; authored functions are C files below `src/`.

## Classification rules

- An EMI type is evidence, not a code verdict. Type `0` may be code, CPU-RAM
  data, or palette data.
- The catalog records `payload_kind`, `code_status`, and evidence separately.
- Exact content identity uses the payload SHA-256. A build target additionally
  includes its load address and entry convention.
- Identical bytes at different load addresses remain distinct targets until
  relocatability and symbol behavior are proven.

## Review loop

```bash
bin/harness scan
bin/harness candidates BATTLE
bin/harness promote "$ARCHIVE_ENTRY" --confirm-code
bin/harness show "$TARGET"
bin/harness next
bin/harness lift "$TARGET" "$ADDRESS"
bin/harness diff "$FUNCTION_SOURCE"
```

Promotion is deliberately explicit. It creates one Splat configuration and one
module source directory. Lifting creates one C-file work item and refuses an
address outside the confirmed payload or an existing lift. Use the generated
draft and disassembly as evidence, then keep the final C89 source readable.

`inspect` is the diagnostic boundary before lifting. Verify its payload,
checksum, load address, Splat configuration, and source directory rather than
compensating for a mapping problem in C.

Promotion creates `internal.h` only when the target source directory does not
already provide one; reviewed target-local declarations are preserved.

`diff` builds the smallest available CMake object and writes comparison evidence
under `out/matching/`. A nonmatch is a normal iteration result; a build or target
resolution failure must be fixed before changing source.

For cross-target navigation, run `bin/harness analysis graph`. It writes
`out/analysis/graph.json` with raw function fingerprints, exact and
relocation-masked duplicate candidates, call edges, PsyQ API usage, and
canonical type usage. Use `bin/harness analysis query TARGET xrefs` for a
focused binary reference view; skipped graph targets mean their normalized
payload is not available locally.

## Candidate replacement loop

Structs, symbols, and functions can be recovered incrementally without making
an unproven shared ABI:

1. Keep an address-only binding in the owning target's `symbols.c` when the
   instruction stream proves an address but not a semantic name.
2. Express a recovered record as a target-local C89 view, preserving raw pads
   and byte/halfword overlays until consumers prove their meaning.
3. Lift one consumer against that binding and run `bin/harness diff` after
   every source change. Use `bin/harness flags` or a bounded candidate search
   only to test compiler shape; generated candidates stay under `out/`.
   Keep the authored lift in C89; do not use inline assembly to force register
   allocation or fill an executable function.
4. Promote a real function or named field only when its bytes match and its
   behavior agrees with the owning spec. Until then, retain the candidate and
   its measured match percentage in the spec ledger.
5. Replace existing declarations one target at a time. Identical payload bytes
   at different load addresses do not share a symbol or struct contract until
   relocatability and runtime behavior are proven.

`bin/harness flags` is an experiment boundary, not a promotion mechanism. For
example, `src/emi/etc/game/00/func_801970ec.c` is 95.31% under the canonical
`-O2` profile but reaches an exact match with `-O2 -fno-schedule-insns`; keep
that result as compiler evidence until the target's original per-function flags
are independently established.

## Source conventions

- `src/exe/` holds source for standalone PS-X executables.
- `src/emi/<family>/<archive>/<slot>/` holds a confirmed EMI module.
- Each module owns an `internal.h`; shared C89/PsyQ declarations belong under
  `include/bof3/`.
- Do not lift PsyQ library routines. Record verified PsyQ symbols in
  `config/symbols/psyq.txt`.

## Promotion quality

A promoted function must:

- compile as clean, maintainable C89;
- retain compact `@behavior` and `@source` trace fields;
- replace `@behavior Pending analysis` with observable behavior rather than
  instruction mechanics;
- add at most one `@see docs/specs/...` path when a durable spec provides
  material context; never link generated state;
- place reusable structures, offsets, and mappings in the owning
  `docs/specs/` concept;
- pass `bin/harness diff`, even when it is not yet an exact match.

## Evidence boundary

The local catalog is generated from user input and is not committed. Retained
research notes live in [specs/](specs/index.md); label unresolved conclusions
as such and keep generated tables out of documentation.
