# Documentation

> Durable BOF3 facts, operating contracts, and reverse-engineering evidence.

## Quick routes

| Need | Read first | Add only when needed |
| --- | --- | --- |
| Repository quick start | [README.md](../README.md) | [setup.md](setup.md) |
| Prepare a workstation and local disc input | [setup.md](setup.md) | [tools.md](tools.md) |
| Binary, archive, EMI-entry, or target identity | [CONTEXT.md](../CONTEXT.md) | [specs/runtime/runtime-layout.md](specs/runtime/runtime-layout.md) |
| Classify, promote, and lift a binary | [reverse-engineering.md](reverse-engineering.md) | [specs/programs/targets.md](specs/programs/targets.md) |
| Iterate on a lifted C function | [matching.md](matching.md) | [tools.md](tools.md) |
| Understand the supported tool roles | [tools.md](tools.md) | [artifacts.md](artifacts.md) |
| Resolve common local failures | [troubleshooting.md](troubleshooting.md) | The workflow page named by the failure |
| Generated versus tracked output ownership | [artifacts.md](artifacts.md) | [AGENTS.md](../AGENTS.md) |
| Browse retained format and runtime evidence | [specs/index.md](specs/index.md) | The owning section index |
| EMI, graphics, STR, or XA format | [specs/formats/index.md](specs/formats/index.md) | The linked format concept |
| Runtime loading, EMI dispatch, frontend flow, or memory layout | [specs/runtime/index.md](specs/runtime/index.md) | The linked runtime concept |
| Executables and confirmed overlays | [specs/programs/index.md](specs/programs/index.md) | [specs/programs/targets.md](specs/programs/targets.md) |
| Archive families, ownership, or duplicate data | [specs/archives/index.md](specs/archives/index.md) | The linked archive concept |
| IDs, encodings, records, tables, characters, equipment, or area data | [specs/data/index.md](specs/data/index.md) | The linked domain concept |
| Algorithm-level recovered behavior | [specs/pseudocode.md](specs/pseudocode.md) | The owning format/runtime/data spec |

`config/splat/` and `config/symbols/` are the tracked layout source of truth.
`out/` is generated and may be deleted and regenerated. The command contract is
`bin/harness --help` and the root `justfile`.
