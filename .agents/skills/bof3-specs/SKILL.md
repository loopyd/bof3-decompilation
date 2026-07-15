---
name: bof3-specs
description: Interpret BOF3 binary facts, formats, runtime layouts, archive and EMI-entry identity, known content, evidence confidence, and discovery methods. Use when classifying a BOF3 payload, determining where code or data lives, mapping an executable or overlay, interpreting EMI types, graphics, CLUTs or audio, correlating duplicate content, or deciding whether a reverse-engineering claim is proven.
---

# BOF3 specifications

Treat BOF3 as independently loaded targets and resources, never as one linked
program. Preserve the shipped identity, extracted bytes, runtime address, and
source owner as separate facts.

## Identity map

| Object | Identity | Analyze/build as |
| --- | --- | --- |
| PS-X executable | shipped path + header + payload hash | normalized headerless load image at header `t_addr` |
| EMI archive | shipped archive path | container only; never code input |
| EMI entry | archive path + slot + payload hash | one extracted raw payload with its reviewed load argument |
| Promoted EMI target | entry identity + load address + entry convention | independent Splat/build/analyzer target |
| Duplicate payload | payload hash plus each load context | separate targets until relocatability and state use are proven |

`SLUS_004.22` and `LOGO.EXE` are separate executables. An archive contains
entries; an entry may be code, data, graphics, audio, or mixed code/data. Type
`0` and plausible MIPS words are candidate evidence, not a code verdict.

Read [payload-map.md](references/payload-map.md) for the compact format and
location map. Read [unknown-blob.md](references/unknown-blob.md) when
classifying or mapping an unreviewed payload. Read
[graphics-evidence.md](references/graphics-evidence.md) before identifying a
texture, palette, CLUT, TIM-like payload, or VRAM destination.

Read [evidence-promotion.md](references/evidence-promotion.md) before promoting
a boundary, name, type, constant, PsyQ identity, or cross-target claim. Read
[cross-binary-correlation.md](references/cross-binary-correlation.md) for
duplicate/shared implementation, ABI, struct, constant, or library evidence.
Read [symbol-type-evidence.md](references/symbol-type-evidence.md) when applying
semantic evidence without losing address traceability.

## Mapping invariant

Map the whole normalized image or extracted entry at its verified load base:

```text
runtime_address = load_base + payload_offset
payload_offset = runtime_address - load_base
```

For a PS-X EXE, read little-endian `pc0`, `t_addr`, and `t_size` at header
offsets `0x10`, `0x18`, and `0x1c`; the load image starts at file offset
`0x800`. For an EMI entry, the type determines whether its load argument is a
CPU address, packed graphics descriptor, or audio selector. Never map an entry
at its first function: leading headers, tables, control words, and padding are
part of the payload layout.

Original bytes and headers outrank manifests, analyzer sessions, and prose.

## Interpretation workflow

1. Name the shipped executable or `ARCHIVE.EMI#slot`; hash and size the exact
   bytes.
2. Determine container, normalized executable, or extracted entry. Never pass
   an EMI archive to Splat, matching, or a raw-code analyzer.
3. Verify the load-base source and require the mapping invariant for every
   proposed code/data boundary.
4. Classify the payload from type, destination, strings, instruction/control
   flow, access patterns, and loader/consumer xrefs. Preserve mixed regions.
5. Search existing catalog, layouts, symbols, sources, and specs before naming
   a target, function, type, resource, or semantic object.
6. Promote only reviewed facts to the narrowest owner. Keep candidates and
   transient analysis under `out/`.
7. Compare duplicates per target: relocation sites, calls, globals, entry
   convention, and overlay lifetime. Share types/ABI only when independently
   proven; never share runtime addresses by assumption.

## Evidence ownership

| Evidence | Owner |
| --- | --- |
| Target identity, payload, and load address | `config/targets/` |
| Binary layouts and segment boundaries | `config/splat/` |
| Authored/shared symbols | `config/symbols/` and target `symbols*.c` |
| Reviewed analyzer replay/types | `config/analysis/` |
| Executable and promoted-overlay C | `src/exe/` and `src/emi/` |
| Shared compiled declarations | `include/bof3/` |
| Stable reviewed findings | `docs/specs/` |
| Extracted bytes, catalogs, analysis, diffs, previews | `out/` |
| Build products | `build/` |

Generated evidence is reproducible support, not the durable owner of a claim.
Do not author source in `out/`, `build/`, or `toolchains/`.

Use `$bof3-docs` when the owning specification is not already known. Mark
unsupported conclusions `INFERRED:` and state the verification path.
