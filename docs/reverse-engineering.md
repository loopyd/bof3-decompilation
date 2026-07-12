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

`diff` builds the smallest available CMake object and writes comparison evidence
under `out/matching/`. A nonmatch is a normal iteration result; a build or target
resolution failure must be fixed before changing source.

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
