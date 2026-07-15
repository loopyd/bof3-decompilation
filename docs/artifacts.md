---
type: Reference
title: Generated artifacts
description: Ownership and regeneration boundaries for BOF3 build and analysis output.
tags: [build, generated]
---

# Generated artifacts

> Regenerable local evidence; never the durable source of binary layout.

| Path | Contents |
| --- | --- |
| `out/extracted/` | Disc tree and unpacked EMI entries |
| `out/binaries/` | Normalized PS-X executable load images and header metadata |
| `out/catalog/` | Raw EMI entry catalog used for promotion review |
| `out/splat/` | Generated assembly, data, and linker artifacts |
| `out/reverse/<target>/snapshot.json` | Normalized per-target stateless analyzer snapshot |
| `out/context/`, `out/lift/`, `out/matching/` | Per-function context, lift, and comparison evidence |
| `out/assets/` | Decoded asset previews |
| `build/src/` | Per-source PSX compiler and assembler objects |
| `build/src/**/*.o.s` | Preserved historical compiler assembly for matching review |
| `out/matching/` | Per-function comparison evidence and summaries |

Tracked Splat configuration and authored symbols live in `config/`; authored C
source lives in `src/`. A partial build must report uncovered executable bytes;
it must not fill them by copying original code.

`just build` compiles every authored source into `build/src/`. Full target
linkage and reconstructed payload verification remain a separate Phase 2
workflow; object compilation must not be presented as a reconstructed EMI
payload or PS-X executable.

The repository does not currently reconstruct the original SLUS CRT or link
layout. Its historical `startup.s` probe and the duplicate LOGO streaming
bridge are excluded from the SLUS validation archive; `LOGO.EXE` remains an
independent target.
