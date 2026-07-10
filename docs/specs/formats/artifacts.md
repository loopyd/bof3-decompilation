---
type: Generated artifact contract
title: Generated artifacts
description: Ownership and regeneration boundaries for BOF3 build and analysis output.
tags: [artifacts, build, generated]
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
| `build/` | Compiler output and coverage reports |

Tracked Splat configuration and authored symbols live in `config/`; authored C
source lives in `src/`. A partial build must report uncovered executable bytes;
it must not fill them by copying original code.
