---
name: bof3-docs
description: "Locate the smallest authoritative BOF3 repository document. Use for documentation lookup, workflow ownership, command help, setup, specifications, artifacts, or troubleshooting."
---

# BOF3 Documentation Navigation

Use the smallest authoritative owner rather than copying workflow rules between
documents.

| Need | Owner |
| --- | --- |
| Repository terms, targets, and directory ownership | `CONTEXT.md` |
| Always-on policy and skill routing | `AGENTS.md` |
| Supported command syntax | `bin/harness --help`, `bin/asmdiff --help`, `bin/permute --help`, `just --list` |
| Discovery, promotion, and lifting lifecycle | `docs/reverse-engineering.md` |
| Function matching | `docs/matching.md` and `$decomp-loop` |
| Generated evidence | `docs/artifacts.md` |
| Stable binary facts | `docs/specs/index.md` |

Treat generated `out/` evidence as support, never as a durable layout or
source owner. For payload interpretation use `$bof3-specs`; for analyzer
evidence use `$psx-rizin`; for C lifting and matching use `$decomp-loop`.
