# Unknown blob decision guide

## Quick path

1. Find provenance: disc path, archive and slot, extraction manifest, size, and
   SHA-256. Stop if the bytes cannot be tied to one shipped object.
2. Check `out/catalog/`, `config/splat/`, `config/symbols/`, and
   `docs/specs/programs/targets.md` for an existing identity.
3. If it is a PS-X EXE, verify the header and normalize bytes after `0x800`. If
   it is inside EMI, extract one entry and retain its TOC type/load argument.
4. Establish the load-base interpretation before decoding. Check
   `runtime - base == payload offset` at several known or proposed addresses.
5. Partition code/data/resources from evidence; do not force the whole blob into
   one segment kind.
6. Retain the investigation under `out/`. Promote only after target identity,
   boundaries, and behavior are reviewed.

For graphics candidates, distinguish file-format headers from raw GPU command
words and VRAM DMA payloads. A raw upload need not be a TIM/PXL/CLT file. Verify
the load destination, transfer dimensions, GPU command or DMA consumer, and
runtime CLUT selection before assigning a format or palette relationship.

A sequence-based exact function match does not validate the mapping base; the
same words may be found while all absolute addresses are wrong. Likewise,
zero-filled static storage does not prove a table is absent or unused: inspect
runtime writers, readers, indirect calls, and xrefs before classifying it as
padding.

## Read-only discovery commands

```sh
bin/harness target list
bin/harness scan
bin/harness candidates <family>
bin/harness target show <target-or-entry>
bin/harness analysis graph
sha256sum <payload>
stat -c '%n %s bytes' <payload>
git check-ignore -v <proposed-path>
```

Use `bin/harness analysis init/query/export` only for an existing target. Native
raw analysis must set MIPS, 32-bit, little-endian, and the verified mapping base.

## Stop conditions

Do not promote when the archive/slot, load-base interpretation, payload bounds,
or first coherent function remains uncertain; when a code claim rests only on
instruction density; when a palette claim rests only on shape/address; or when
duplicate bytes still have unreviewed relocations or target-local references.

Route unresolved format questions to `docs/specs/formats/index.md`, runtime
questions to `docs/specs/runtime/index.md`, archive ownership to
`docs/specs/archives/index.md`, and verification procedure to
`docs/specs/methods/index.md`.
