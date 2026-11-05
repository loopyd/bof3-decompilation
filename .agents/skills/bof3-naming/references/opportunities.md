# Naming investigation

Start with `bin/harness naming list TARGET --json`; `--limit` limits display only.
Use `--kind symbols` for maps, `references` for lexical locations, `identifiers`
for arguments/locals and `files` for claimed source paths. Missing inputs and
unsupported syntax remain explicit blockers. Fresh indexes can supply additional
call/data references through `bin/harness analysis query`; refresh only when authorized.

Resolve function identity from target-qualified `@source` and reviewed boundaries.
Separate functions, data, support bindings, declarations and source paths. A raw
prefix, filename or neighboring named symbol proves no semantic role or storage kind.
Trace callers, branches, fields and consumers against original instructions. State
what each observation proves and which role, extent or owner remains unknown.

For data, establish storage section, finite extent, element width, initializer or
runtime consumer, and target ownership independently. Equal overlay addresses do
not establish shared storage. Promote to shared maps only after checking every
composing target and each affected consumer. Preserve `INFERRED:` observations
until their missing verification is available.

A useful result is an evidenced name or an explicit unresolved row with the next
bounded investigation. Preserve the selected queue and reasons for deferral in
its existing change or campaign notes. No generated account or terminal report
confers semantic acceptance or edit authority.

## Duplicate sets

Identical bodies across targets carry one role, so a set that is named in one
overlay and still `func_<ADDR>` in another is one transaction, not a per-target
guess. Find the sets in `duplicate_groups` and `duplicate_members`
(`reviewed_sha256` plus `reviewed_size`) and rank the half-named ones. A member's
name is evidence only when its bytes hash equal to the named copy's, so the group
sha carries the proof and resemblance does not.

The parent decides every name and hands one lane per affected target a pinned
list, because lanes cannot see each other and independent choices re-fork the
set. Same body means same role; suffixes exist only for collisions inside one
target, so letter them in ascending address order and keep an already-named copy
unchanged unless two spellings describe the same body, in which case unifying
them is the point of the transaction. Files stay separate: each target compiles
its own copy, and a shared filename needs the collision form used elsewhere.
Each affected target still gates once.
