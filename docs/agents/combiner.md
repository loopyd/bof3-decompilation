# Function consolidation

The combiner groups cohesive implementations into readable C89 translation units;
it does not merge function identities or extract macros. Macro discovery and
semantic acceptance remain in [macros](macros.md); symbol changes belong to naming
and representations to types. See the [active plan](../plans/autonomous-bof3-decompilation.md#multi-function-consolidation)
for rollout and unfinished gates.

## Current capability

`sh bin/combiner inspect-source src/bof3/ui/advancePanelXTo17.c` reads one explicit
source without building, refreshing the index or writing. `harness.domain.functions`
owns `parse_function_records` and `select_function_record`; `harness.combiner`
owns inspection and CLI adaptation. Source claim enumeration and address-selected
metadata now support grouped files; the reverse index derives each member's own
lifecycle and attributes direct macro occurrences by implementation range. This is
not full consumer migration: native matching, metadata repair, source-renaming and
macro/type transactions still reject grouped inputs. Do not consolidate production
sources until all gates are implemented.

Each implementation has its own immediately preceding comment containing one
`@source` and nonempty `@behavior`. Progress tags remain an atomic
`@status`/`@match`/`@residual` set, scoped to that record; missing progress means
unproven, not exact. The manifest supplies target identity, maps/Splat the compiled
symbol and original range. Neither file basename nor a macro argument proves identity.

```c
/* @source 0x801E31C4
 * @behavior Adds 32 to panel x, then clamps to 320 and clears state above 320.
 */
PANEL_ADVANCE_X(advancePanelXTo320, 320)

/* @source 0x801E2CDC
 * @behavior Adds 32 to panel x, then clamps to 17 and clears state above 17.
 */
PANEL_ADVANCE_X(advancePanelXTo17, 17)
```

Inspection recognizes ordinary ANSI C89 definitions and standalone template
invocations, not expansion semantics. It reports source hash, one-based start line,
half-open Unicode-character metadata/implementation ranges, spelling and claimed
status, with `native_verified:false`, `coverage_verified:false` and
`write_authorized:false`. Inspection does not enumerate untagged helpers or prove
that a macro emits exactly one function. Template spelling
names the invoked macro, not its generated function. Duplicate addresses, malformed
progress, unattached/nested metadata, prototypes, conditional source, continued lines
and unsupported declarators reject rather than guess. Tags in strings are ignored;
data declaration tags without behavior do not identify a function. Leading metadata
before includes is a legacy file convention, not the new attached-record format.
Code between tagged implementations must end with a declaration semicolon; an
untagged helper ending in `}` is not assumed to establish a safe boundary. Add its
own leading metadata or defer unsupported source for review. Implicit-int definitions
also require an explicit return type before inspection.

Read-only enumeration uses `collect_lift_metadata` / `select_lift_metadata` and
retains invalid progress as invalid rather than hiding a function. Strict inspection
still rejects malformed progress. Legacy single-function metadata before includes or
split across adjacent comments remains readable; combined files require attached
records. Unscoped tag readers and exact-progress repair reject multiple records,
including incomplete member tags, instead of selecting or overwriting the first.

Reviewed source expectations retain every boundary address: singleton values remain
integers and grouped values are sorted tuples. Scans require complete agreement and
reject duplicate owners; they do not choose the first path. Layout promotion and
function-qualified status caching remain pending. Index v15 invalidates older derived
semantics; preserve the old index before rebuilding. A rejected candidate never
replaces it, and an older schema is historical evidence, not current acceptance.

Macro association uses authored ranges even when the analyzer omitted a sibling;
that sibling's uses remain unresolved, never borrowed by the sole indexed function.
Prologue/header/definition uses remain contextual. This does not yet migrate macro
consumer coverage or type-use identities. Native resolve/compare and status preflight
reject grouped files before comparison/cache reuse; transaction preparation, proposed
C images and filename-changing naming facts retain corresponding guards.

`bin/cc` now passes its one complete maspsx translation through
`build.translation` before assembly. Ordinary sources pass through unchanged;
grouped lifts require attached records, exactly one explicit source owner and
complete map/Splat-resolved assembler membership. CMake tracks the adapter,
partitioner and consumed ownership inputs. This low-level producer retains the
selected GCC and ordered flags; it does not prove a consolidation preserved each
member's PRE profile. Those profiles currently live in path-keyed
`config/compiler/object-flags.cmake`, not function tags. Unsupported compiler/flag
annotations anywhere in grouped-source comments reject rather than silently fall
back. Profile migration must freeze old paths/settings before moves, require one
compatible effective profile and reproduce it at the new path.

`build.sections` partitions output; `match.placement` is still outside production
matching. The kernel rejects unsupported text subsections/attributes, unplaced allocated sections,
incorrect symbols/ranges and non-ELF32 little-endian MIPS inputs. `match.flow`
checks reachable branches and delay slots, conservatively rejecting unproved
indirect/exception transfers. This proves isolation, not original-byte fidelity.
`match.execution` supervises every native command under one caller-owned absolute
deadline and output bound. Linking validates a private snapshot before confined,
exclusive script/ELF publication; the pair is not atomic. Failure retains scratch
and partial outputs, including when descendant cleanup is unconfirmed. Concurrent
artifacts are never replaced. These kernel fixes pass disposable native probes
and independent review, not grouped-source acceptance. A scratch two-panel unit
also passes original-byte/instruction comparison after one configured GCC run.
Profile compatibility, full producer/include/toolchain freshness, shared-data
proof, matcher instruction selection, complete member coverage and function-scoped
caching remain open; do not bypass existing guards. Current CMake ownership
dependencies are conservative across targets, not a performance optimization.

## Planned discovery and ranking

The cleanup node is **combiner**, with explicit discovery, assessment, audit and
transaction modes under one concern skill. These modes beyond inspection are queued,
not implemented commands. Candidates require at least two distinct functions and
two existing source files; a single already-cohesive file is not a fabricated win.

| Method | Evidence required before selection |
| --- | --- |
| Macro family | Same resolved body-emitting template and coherent operation; retain complete consumer inventory, including other targets |
| Functional subcategory | Corroborated behavior and a meaningful common responsibility, not basename/directory similarity alone |
| Shared state or type | Proven use of the same target-owned state/representation with a common lifecycle; mere global/type co-occurrence is insufficient |
| Helper/call cluster | Reviewed caller/helper cohesion and preserved linkage; exclude generic utility hubs and unrelated callers |

Machine ranking binds method, membership, source/index/config hashes and explicit
size/top-N limits. Prefer stronger cohesion and useful file reduction, then a stable
selector tie-break; penalize namespace/dependency growth and oversized miscellaneous
files. A separate AI assessment asks **“would a human do this?”**: can the file have
one clear name, improve navigation/local reasoning, preserve useful boundaries and
avoid hiding different lifecycles? Record accept/reject/defer with evidence and
counterevidence. Score, string similarity and file-count savings never authorize edits.
Overlapping candidates compete for the same functions; freeze the selected set and
original attempt/deadline budgets before trying configurable top N.

## Consolidation safety contract

- Initial combinations are target-local. The 14 panel wrappers span three independent
  images with repeated symbols: consider one coherent file per target, not a single
  14-function object. Shared templates remain separately owned; sharing C ownership
  across targets is a distinct later design, never inferred from equal bytes.
- Preserve each selector, map address, original Splat range, ABI, visibility and
  attached metadata. Many function boundaries may reference one source only after
  registry/layout validation explicitly checks that complete relation.
- Resolve project defaults and every existing compiler/flag override through the
  compiler owner. A translation unit cannot mix GCC versions; reject incompatible
  profiles rather than drop overrides, force GCC 2.7.2 or install compilers. Preserve
  include order, preprocessor state, local statics, data/rodata and relocation binding.
- Evaluate deterministic per-function generated compilation units before adopting
  whole-object relinking: they may preserve noncontiguous originals with less native
  risk. Projections need full dependency/semantic review, original per-function
  settings and generator/content hashes in freshness/receipts; they are disposable
  build products, not new authored owners or permission to duplicate shared state.
- Native comparison must select each symbol's range and relocations, not compare
  the entire `.text` prefix at every function address. Prove every affected function
  before/after and all touched target builds/maps/layouts, including noncontiguous
  originals and inter-function calls. No first-function cache or lifecycle reuse.
- Preparation freezes all old/new paths, source claims, compiler configuration and
  affected consumers. Independent justification precedes the consolidation transaction;
  native gates and a distinct final review follow it. Reuse owned-image recovery,
  writer exclusion, external pins and concurrent-work preservation; deletion/move
  support must be demonstrated, not assumed from header-edit transactions.
- Failed or cancelled work retains original evidence and budget; restore only proven
  owned PRE after confirmed writer termination. Discovery grants no rename, macro,
  type, report repair, index rewrite during pinned work or acceptance authority.

## Validation

Run existing source/claim/layout, compiler/build/matching, index/status, naming/type/
macro, transaction/recovery and cleanup routing checks for each migrated owner.
Disposable multi-record and native fixtures characterize new paths without adding
unrequested regression tests. The first production pilot needs independent review,
all-member byte equality, rollback/cancellation evidence and refreshed references.
