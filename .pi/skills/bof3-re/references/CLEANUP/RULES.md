# Cleanup rules — where cleanup applies and where it never does

Never turn an audit into edits, a docs repair into a source refactor, or a naming transaction into a lift/matching experiment. Cosmetic and evidence-preserving only: source filename changes require a complete metadata-backed naming transaction; never change behavior or a byte-match.

## Evidence gate

Retain a name only with exact target-local address/layout plus
two independent corroborators: two consistent local access/call sites; one
local site plus a reviewed Rizin annotation; or proven local layout/dispatch
table plus consistent uses. Decompiler name, duplicate hash, string, comment, or one call
site alone: insufficient. Name only what evidence proves; keep
`D_XXXXXXXX`/`unk_XX`/`field_XX` when meaning is uncertain. Name a proven
data region by content class: strings, pointer/handler table, or struct
layout plus consistent consumers; a `data-scan` ref count is a lead, not
evidence.

A rename must not change width, signedness, pointer depth, volatility, ABI,
storage, array extent, packing, code shape, control flow, matching aids,
compiler flags, or binding addresses.
## Authority ceiling

- `symbol`/`type`: edit one selected target only — `symbols.txt` (one
  spelling, unchanged address), target `internal.h`/`symbols.c`, direct
  same-target references. Keep the map sorted; no aliases; never edit
  generated `symbols/psyq.c`.
- `docs`: edit only named existing files under `docs/`, `AGENTS.md`,
  `README.md`. Delete dated lift counts, transient rankings, `out/`
  snapshots, dead links, duplicated instructions — never relocate them.
  Changelog/history entries stay. Durable facts → `docs/specs/`; agent
  policy → `docs/agents/`; scoped work → `docs/plans/`.
- `audit`: report each finding as `path`, current contract, evidence, smallest safe repair, validation, human-approval needed. Large `internal.h` and raw compiled-symbol spellings are not drift; source filenames may be semantic because metadata owns identity.

## Identity contracts — never touch

- Every lift carries parsable function-level `@behavior` and `@source`; the `@source` tag is the address authority and no filename fallback exists. Rename a source file or compiled entry function only through a complete naming transaction (evidence gate). Never rename a Splat function boundary address.
- Never drop `@behavior`/`@source`/`@kind` tags or evidence comments;
  correct stale tags in place. Tags are written `/* @source 0x... @kind
  ... */` — `//` comments break gcc-2.6.3 variant objects.
- Never move target directories or alter `source_dir`, manifests, load
  addresses, Splat boundaries, compiler/toolchain files, SDK
  maps/declarations, shared/public headers, `src/shared/`, `out/`, `build/`,
  `toolchains/`.
- Reorganization = audit finding + `docs/plans/` plan + explicit user
  approval, then a separate task. Crossing these boundaries: report
  plan/blocker, no edits.

## Transaction

1. Refuse overlap with an already-modified candidate file unless the parent
   names that exact edit as part of this transaction.
2. Naming: record old spelling, unchanged address/layout, binding location,
   target-local references, corroborating evidence; update map, declaration,
   binding, same-target references together. For data, also record content
   class as a `@kind: bss|rodata|string|table` comment beside the
   declaration. The map plus `WEAK_SYMBOL_AT` binding are the only address
   authorities, so a later rename or relocate stays one transaction.
3. Docs: find the owning current fact; remove/correct only the stale claim.
4. Audit: classify — safe local repair, needs scoped plan, or blocked by
   ownership/evidence. No manufactured abstractions.
5. Verify: no target-owned reference keeps the old spelling, no unrelated
   target changed, every edited doc link resolves.