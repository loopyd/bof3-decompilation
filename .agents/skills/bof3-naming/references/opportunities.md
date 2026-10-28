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
bounded investigation. Preserve the selected queue and reasons for deferral in its
existing change or campaign notes. No generated account or terminal report confers
semantic acceptance or edit authority.
