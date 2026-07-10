# Documentation

> Durable BOF3 facts, operating contracts, and reverse-engineering evidence.

| Need | Read |
| --- | --- |
| Prepare a workstation and local disc input | [setup.md](setup.md) |
| Classify, promote, and lift a binary | [reverse-engineering.md](reverse-engineering.md) |
| Resolve common local failures | [troubleshooting.md](troubleshooting.md) |
| Browse retained format and runtime evidence | [specs/index.md](specs/index.md) |

`config/splat/` and `config/symbols/` are the tracked layout source of truth.
`out/` is generated and may be deleted and regenerated. The command contract is
`bin/rebof3 --help` and the root `justfile`.
