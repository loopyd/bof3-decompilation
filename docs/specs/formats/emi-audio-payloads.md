# EMI Audio Payload Semantics

This document tracks audio-side payload semantics carried inside EMI archives.

Use `emi.md` for container structure and shared type mapping. Use `runtime/audio-system.md` for PSYQ call-path and handler-level behavior.

## Status

- Confidence: medium
- Scope:
  - Type `6`, `7`, `10` proven paths
  - Type `8` and `9` as constrained leads

## Current Type Mapping

Current supported meanings:

- type `6`: VAB header (`VH`)
- type `7`: VAB body (`VB`)
- type `10`: sequence payload (`SEQ`)
- type `8`: auxiliary bank-local audio/control payload (format unresolved)
- type `9`: sequence-adjacent or auxiliary payload path (locally unresolved)

Primary runtime reference:

- `docs/specs/runtime/audio-system.md`

Container reference:

- `docs/specs/formats/emi.md`

## Bank Id And Load Arg Usage

For audio-side types, the EMI load argument (`ram_ptr`) is often a logical bank selector rather than a CPU RAM destination pointer.

Implication:

- classify audio entries by bank-id behavior and handler path, not by pointer-like interpretation alone

## Validation Checklist

For an audio payload claim to be high confidence, prefer proving:

- handler path in loader/runtime docs
- payload signature where applicable (`VH`/`VB`/`SEQp`)
- bank-id mapping behavior from manifests and call path

## Open Points

- exact payload schema and runtime use for type `8`
- concrete shipped local examples (if any) for type `9`
- per-overlay audio helper behavior beyond current SLUS-side handler coverage
