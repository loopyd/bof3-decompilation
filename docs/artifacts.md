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
| `out/catalog/` | EMI entry catalog, duplicate groups, and lift records |
| `out/splat/` | Generated assembly, data, and linker artifacts |
| `out/ghidra/` | Ghidra project/export data |
| `out/work/` | Per-function drafts and comparison output |
| `out/assets/` | Decoded asset previews |
| `build/default/bof3.exe` | Reconstructed main PS-X executable |
| `build/default/artifacts/raw/` | Raw images for targets with confirmed load layouts |
| `build/default/artifacts/compiled/` | Intermediate object sets for targets without confirmed raw layouts |
| `build/default/artifacts/metadata/artifacts.json` | Registered target and build-stage manifest |

Tracked Splat configuration and authored symbols live in `config/`; authored C
source lives in `src/`. A partial build must report uncovered executable bytes;
it must not fill them by copying original code.

`just build` builds every registered artifact. A target may emit a raw blob only
after its load address and object placement are confirmed. Until then, its
compiled `.a` is an intermediate object set and must not be presented as a
reconstructed EMI payload or PS-X executable.
