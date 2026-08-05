# Cleanup rules — where cleanup applies and where it never does

Never turn an audit into edits, a docs repair into a source refactor, or a
naming transaction into a lift/matching experiment. Cosmetic and
evidence-preserving only: never rename/move `func_XXXXXXXX.c` files, never
change behavior or a byte-match.

## Evidence gate

Retain a name only with exact target-local address/layout plus
two independent corroborators: two consistent local access/call sites; one
local site plus a reviewed Rizin annotation; or proven local layout/dispatch
table plus consistent uses. Decompiler name, duplicate hash, string, comment, or one call
site alone: insufficient. Name only what evidence proves; keep
`D_XXXXXXXX`/`unk_XX`/`field_XX` when meaning is uncertain.

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
- `audit`: report each finding as `path`, current contract, evidence,
  smallest safe repair, validation, human-approval needed. Large
  `internal.h`, raw address spellings, address-based filenames are not drift.

## Identity contracts — never touch

- Never rename/move `func_XXXXXXXX.c`, rename a raw lifted entry function, or
  rename a Splat function boundary.
- Never drop `@behavior`/`@source` tags or evidence comments; correct a
  stale tag in place.
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
   binding, same-target references together.
3. Docs: find the owning current fact; remove/correct only the stale claim.
4. Audit: classify — safe local repair, needs scoped plan, or blocked by
   ownership/evidence. No manufactured abstractions.
5. Verify: no target-owned reference keeps the old spelling, no unrelated
   target changed, every edited doc link resolves.