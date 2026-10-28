# Harness ownership

The harness lists current facts, checks mechanical consistency and runs bounded
native tools. Concern skills investigate, edit and review source, maps, layouts
and metadata directly. Start with the [documentation index](../INDEX.md) for the
skill and contract that own a request.

## Domain packages

All paths below are relative to `tools/python/harness/`.

| Package | Owns | Entry points |
| --- | --- | --- |
| `naming/` | spelling debt, source identifiers, map consistency and native consumers | `bin/harness naming list/check` |
| `types/` | declarations, representation facts, uses, storage leads and checks | `bin/harness types list/check` |
| `macros/` | definitions, uses, repetitions, original-byte leads and checks | `bin/harness macros list/check` |
| `combiner/` | current source members, relationships, effective profiles and checks | `bin/harness combiner list/check` |
| `decomp/` | metadata reports, lift inventories, pinned missions, diagnosis and audit | `bin/harness source metadata list/check`, `bin/harness decomp` |
| `match/` | individual/grouped native comparisons, result validation and disposable status cache | `bin/harness lift asm-diff`, `byte-match`, `status` |
| `build/` | compiler settings, native production, receipts and hybrid payload delivery | `bin/harness build`, `build payload`, compiler adapters |
| `analysis/` | shared reverse index and cross-domain factual queries | `bin/harness analysis index/query` |
| `domain/` | manifests, target identity, binary/layout, source claims and declarations | imported by their consumers |
| `runtime/` | emulator staging, launch, completion and receipts | `bin/harness runtime` |
| `reconstruction/` | original-only inputs, native generation and independent comparison | `bin/harness reconstruction` |
| `toolchain/` | pinned setup, native builds and installation verification | `bin/harness setup/doctor` |
| `common/` | shared CLI, digests, confined files, paths, processes and native safety | direct imports from the actual owner |

Package initializers are inert. Import the actual owner rather than a compatibility
facade. The cross-domain index is one derived database. Index-dependent commands
refuse stale or missing facts; refresh explicitly with `bin/harness analysis index`.
List actions do not refresh indexes, compile or write reviewed files. Check actions
write only disposable native outputs. A display limit never reduces findings.

`domain.functions` parses complete attached function metadata and lexical ranges.
`domain.claims` resolves every member and rejects ambiguous ownership. Function
identity comes from the manifest, target-qualified address, maps and reviewed
layout. A filename or first-record fallback cannot identify a grouped member.
`common.lexicon` owns shared comment and literal masking.

`domain.declarations` and `domain.c_context` parse namespace-qualified entities
and their source occurrences. `types.representation` resolves explicit identity
aliases. Forward declarations retain provenance; pointer/array aliases do not
inherit pointee layout. Constraints retain contradictions and unknowns instead
of inventing layouts. Rebuild the versioned index when derivation changes.
Representation facts and storage leads do not establish semantic correctness.

`decomp.consumers` finds current identifier references and complete source groups.
Type checks close referenced declarations and their dependents; macro checks
close definition dependencies and consumers. Unresolved or cross-target consumers
remain explicit unavailable coverage. Check their owning targets before accepting
an edit that affects them.

## Function metadata

`bin/harness source metadata list TARGET [--limit N] [--json]` lists authored
function records and labels any cached measurement as fresh, stale or absent.
It does not compile sources or update the disposable status cache. The output
identifies truncation and retains findings for the complete selected target.

`bin/harness source metadata check TARGET [TARGET@0xADDRESS ...] [--json]`
compares current native results with the selected claims. Without selectors it
checks the selected target. Selecting a grouped function checks every member in
its source. Complete original-range equality is required for exact status.
Checks return 0 for passing coverage, 1 for findings or unavailable evidence,
and 2 for an invalid request. Both commands leave reviewed files unchanged.

The [reverse engineering skill](../../.agents/skills/bof3-re/references/metadata.md)
owns edits to `@source`, `@behavior`, `@status`, `@match` and `@residual`. Record a
successful match only after a current live byte gate passes, then check the edited
record. A cached result or authored status does not replace that live gate.

## Policy versus mechanism

The naming, type, macro, combiner and metadata interfaces expose only `list` and
`check`. They provide no prepare, apply, rank-account, promotion or rollback
protocol. Their concern skills own selection, semantic decisions, direct edits,
independent review and restoration through the shared
[editing and recovery contract](../../.agents/skills/bof3-re/references/editing-recovery.md).
Keep frozen campaign scope, partial-result constraints and unfinished duties in
the owning change or campaign. Historical reports remain history.

The read-only cleanup context router selects the skill for one canonical request:

| Request | Skill |
| --- | --- |
| `naming-opportunity TARGET ID` | `bof3-naming` |
| `type-opportunity TARGET ID` | `bof3-types` |
| `macro-opportunity TARGET ID` | `bof3-macros` |
| `combiner-opportunity TARGET ID` | `bof3-combiner` |

Pass one form to `bin/harness agent context cleanup`. The router validates a known
target and printable opaque ID, retains that ID and loads the selected skill.
It neither queries opportunities nor grants edit authority. The skill must check
candidate existence, membership, freshness and the caller's scope. Canonical
re-derivation rejects inconsistent request fields or a different skill selection.
Identity forms such as `type TARGET OLD -> NEW` select naming; representation
refinement belongs to the type skill. Use `docs PATH...` for documentation and
`audit-target TARGET` for naming inspection. Cleanup context remains importable
under the entrypoint's stdlib-only `-S` bootstrap.

The harness never launches Codex or model processes. The
[lift-loop skill](../../.agents/skills/bof3-lift-loop/SKILL.md) uses active-session
missions and independent review. `decomp.diagnosis`, `audit`, `missions`, `scope`,
`gates`, `execution` and `evidence` retain their deterministic native gate duties.
`match.validation` validates actual comparison payloads, full ranges and pinned
partial baselines. Passing a payload check does not grant semantic acceptance.

## Native build and comparison

`build.compiler` resolves attached function settings, configured object overrides
and project defaults. Ordered compiler arguments remain significant. Grouped
sources require compatible effective settings for every member. The combiner
skill records PRE settings and checks that source movement preserves them.

`build.arguments`, `invocation`, `dispatch` and `driver` separate source operands,
forced includes, generated artifacts and compiler stages. Dispatch captures current
inputs and configured program observations, checks freshness and protects source
inputs from output collisions. `build.execution` records actual production edges;
`build.receipts` binds native products to that invocation. A producer receipt
proves provenance, never equality or permission to reuse a result.

`match.members` resolves current source ownership, attached records, complete
reviewed Splat ranges, original bytes, canonical map bindings and data placements.
`match.grouped` compiles the whole translation unit and compares every member at
its original placement. It rejects a differing neighbor when reporting an exact
member and rejects failed required initialized data even for partial members.
Unsupported BSS, computed jumps or placements remain unavailable. No external
preservation record or routing environment selects a different build path.
`match.images` captures immutable emitted code and reviewed initialized data
within live native guards. `build.payload` places those ranges in the shipped
file and delegates resident archive replacement to the Rust EMI owner.

`build.dependencies` captures literal transitive includes, forced inputs and
negative lookups. `build.programs`, `runtime` and `observations` describe bounded
configured tool inputs and runtime candidates. These observations do not prove
complete active preprocessing, imported Python code or effective compiler-backend
selection. See [native execution](../../.agents/skills/bof3-re/references/native-execution.md)
for retained closure limits, process bounds and cleanup obligations.

`common.process` owns supervised native execution. Unconfirmed cleanup propagates
`ProcessCleanupError` without another comparison, retry or fallback. `common.lease`
and the build writer owners serialize shared writes. `common.inputs`, `paths`,
`observation`, `directory`, `lookups` and related helpers retain confined reads,
identity checks, mutation watches and bounded directory observations. Endpoint
membership does not prove uninterrupted history.

`common.files`, `rename`, `artifacts` and `quarantine` retain guarded publication
and native output recovery. They are used by current build, runtime and lift
owners; they do not edit concern-owned source on a skill's behalf. Preserve failed
outputs until their owner establishes actual state. `common.git` and `workspace` support current mission and file/index safety. Raw index state and unrelated
dirty work remain outside a concern edit's restoration scope.

`domain.cache` owns complete manifest discovery and pass-local reuse. Each load
revalidates claims, including cache hits. Captured readers supply immutable samples;
those samples alone do not prove physical containment or freshness. Confined input
batches retain canonical paths, regular-file checks, linkage and exact contents.
Missing, empty, unreadable and malformed inputs remain distinct.

Ordinary `lift status` uses a disposable fingerprinted cache and batch-builds
eligible misses per target. It reloads ownership after builds and retains scoped
fallbacks. Grouped comparisons bypass ordinary cache and batch-object reuse.
A status summary is not acceptance evidence; use current live gates for acceptance.

## Hybrid game-file output

Build a complete shipped file with freshly compiled exact ranges and original
bytes for the remaining ranges:

```sh
bin/harness build payload exe/logo --output-dir out/payloads/logo --json
bin/harness build payload emi/etc/game/01 --output-dir out/payloads/game01 --json
```

Each output directory must be new and under `out/`. The command uses existing
extracted originals, normalized target binaries and configured native tools.
Ordinary `bin/harness build TARGET` continues to produce objects.

`payload.bin` contains the whole original-based executable or resident entry.
Executable output retains its PS-X header. `report.json` records fresh source
comparisons, provenance, disjoint compiled and retained intervals, byte counts
and complete file equality. At least one nonempty compiled range must pass the
live byte comparison. Stored progress does not qualify a range; differing code
retains original bytes. A group contributes only when all members and placed
initialized data match. Invalid metadata, overlapping ownership and unsupported
required placement fail the operation.

For an EMI target, `archive.EMI` replaces the correct original entry through
the Rust EMI owner. Verification compares the entire archive and re-extracts
the selected slot into `entry.bin`. Original inputs remain unchanged. Failed
operations may retain diagnostics but do not publish `report.json`.
These are hybrid outputs; their compiled coverage does not imply a source-only
rebuild, disc-image mastering or runtime acceptance.

## Validation gates

Run focused existing tests for changed owners, Ruff, documentation drift and
`just check`. `just check-all` also runs all Python units. Tests assert behavior
or parsed structure rather than literal skill prose. Report failed, skipped and
unavailable checks separately.

Use `bin/harness lift asm-diff` for instruction comparison and `lift byte-match`
for bytes. Exact acceptance requires equal sizes and every byte of the complete
reviewed original range, including required grouped members and data. Run required
Splat, binding, source and companion checks for the selected work.

Use `bin/harness source symbols check` after map edits and
`bin/harness source symbols normalize [TARGET] --write` when authorized. The
existing symbol baseline records reviewed raw-name debt; updating it requires
acceptance of that debt, not merely a failing check. Native checks, semantic review
and independent acceptance remain necessary after passing Python tests.
