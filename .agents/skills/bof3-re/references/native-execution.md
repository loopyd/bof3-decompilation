# Native execution

## Authority and profile

Autonomous parent has standing authority for bounded index/analysis refreshes,
independent reviews, native compile/compare gates and accepted feature-completion
commits. No push/release, install, policy weakening or retry after denial. Ordinary
one-function work still needs explicit commit approval. Preserve original scope,
pins and consumed bounds; refresh only at safe checkpoints, never during pinned
transactions. Review payload includes relevant source/diffs/instructions/metadata/
original bytes, never credentials or unrelated media.

Use actual session tools. No shell/MCP/wrapper Codex launch, model SDK/API, detached
runner or substitute Pi process. Missing host capability blocks dependent gates;
worker output cannot self-accept. Ordinary capacity retries must retain original
bounds; an approval denial is not a capacity failure.

Resolve effective compiler/flags before native work: attached function settings,
then `BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` in
`config/compiler/object-flags.cmake`, then project defaults. Managed variants live
in `config/compiler/variants.json`; generated compile database and `bin/cc` carry
selection. Grouped functions require compatible effective profiles. Changing
execution route never changes compiler/flags or substitutes host GCC.

## Host capability

Canonical driver, `cpp`, `cc1` are static i386 ELF. Known Codex sandbox failure is
`SIGSYS`, shell exit 159, even for `gcc --version`; nested `bwrap` cannot remove
inherited syscall restrictions. This is capability failure, not C evidence.

On that verified host/compiler, parent requests exact bounded native gate through
exposed reviewed escalation before baseline compilation. Where `exec_command`
exists, use `sandbox_permissions: "require_escalated"`. Do not invent this API on
another host. Prefer pinned diagnosis/audit; direct scoped diff/byte gates need
same reviewed capability. Unavailable worker returns `unverified`, null score.
Different host/compiler: establish capability once. Python inspection stays
sandboxed. Standing authorization removes extra conversational checkpoint, not
per-action tool approval. Denial/unavailability blocks gate; continue unaffected work.

Diagnosis/audit inner installed `bwrap` keeps root/source/Git/evidence read-only,
separates network/namespaces, permits generated build/comparison/binding/Splat
outputs and fresh scratch, masks home Codex/Pi directories. Outer restrictions
still apply. No install, self-elevation, permission edits or alternate wrapper
after denial. This is cooperative confinement, not protection against privileged
writers/hostile aliases; other credential locations are not inventoried.
