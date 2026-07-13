---
name: bof3-rizin
description: Maintain persistent Rizin or radare2 analysis projects for BOF3 binaries, replay reviewed symbols and C types, inspect strings and xrefs, and export deterministic evidence. Use for Rizin, rz-ghidra, radare2, raw PSX MIPS analysis, symbol renaming, type application, or analysis-project maintenance in this repository.
---

# BOF3 Rizin Analysis

Use Rizin when available and radare2 as the compatible local fallback. Treat both
as evidence workbenches: original bytes and tracked Splat layouts remain
authoritative.

## Workflow

1. Read `CONTEXT.md` and identify one executable or promoted EMI target.
2. Run `bin/harness analysis doctor` before choosing an engine. Never assume
   Rizin, rz-ghidra, or a decompiler plugin is installed.
3. Initialize or refresh the generated project with
   `bin/harness analysis init <target>`. Projects belong only under
   `out/analysis/projects/`.
4. Put reproducible, reviewed renames and type applications in the target's
   tracked replay file under `config/analysis/`. Put shared analysis-only C89
   type declarations in `config/analysis/bof3_objects.h`.
5. Re-run initialization after changing tracked inputs, then export with
   `bin/harness analysis export <target>`. Do not hand-edit exports.
6. Use `bin/harness analysis query <target> ...` for focused strings,
   functions, xrefs, and type-placement questions.
7. Run `bin/harness analysis graph` to fingerprint every available promoted
   binary, derive call edges, resolve PsyQ callsites, and group exact-byte or
   relocation-masked duplicate functions. Read `out/analysis/graph.json` for
   the detailed graph; skipped targets are reported when their normalized raw
   payload is not present locally.
8. Verify every promoted fact against disassembly or raw bytes. Move durable
   findings into `config/splat/`, `config/symbols/`, source declarations, or
   `docs/specs/`; leave hypotheses in generated evidence.

## Safety and ownership

- Load raw binaries as little-endian 32-bit MIPS at the target's verified load
  address. An EMI archive is never the input; use its extracted raw entry.
- Keep separate projects for separate targets, even when payload hashes match.
- Use canonical spec names such as `ItemObject`, `WeaponObject`,
  `AbilityObject`, and `EnemyObject`. Record an upstream alternate label in a
  spec; do not create compatibility typedefs.
- Analyzer-created names, guessed types, and decompiler output are provisional.
  Only reviewed facts enter tracked replay scripts.
- Never write generated project state outside `out/` or generated analysis into
  authored source directories.
- Do not lift PsyQ library code or use decompiler output as match authority.

## References

- Read [commands.md](references/commands.md) when invoking Rizin/radare2,
  importing types, managing projects, or diagnosing version differences.
- Read [evidence.md](references/evidence.md) before promoting names, types,
  addresses, IDs, strings, xrefs, or struct layouts into tracked artifacts.

When a command differs between engines, use the harness adapter. If the adapter
does not cover it, record the detected engine/version and use the documented
native command without changing repository ownership rules.
