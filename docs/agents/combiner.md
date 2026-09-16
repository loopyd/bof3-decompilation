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
lifecycle and attributes direct macro occurrences by implementation range. Naming's
single-row metadata and partial-status readers select the addressed record, matching
bulk inspection for valid, unambiguous records. This does not prove naming readiness
or atomic read consistency: ordinary native matching, preflight metadata repair, source-renaming and
macro/type transactions still reject grouped inputs. Do not consolidate production
sources until all gates are implemented.

`naming.metadata` owns bulk and single-row lifecycle inspection. Ambiguous function
owners, unreadable claimed C files, malformed record structure and failed target
resolution reject rather than become absent sources. Bulk context construction
fails; single-row inspection returns an invalid diagnostic. Malformed progress
remains a per-record blocker. A missing function claim in a successfully inspected
inventory is distinct from an inspection failure; these facts grant no native or
transaction authority and do not establish an atomic source snapshot.

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

Inspection captures at most 16 MiB through confined held descriptors, rejecting
symlinks, multiple links and nonregular inputs before reading content. It parses
and hashes the same captured bytes; the source name is not reopened after a path
precheck. The shared `InputBatch.read(single_link=True)` policy is opt-in; other
callers retain their existing hardlink policy. This binds one input generation,
not current source freshness after return or an atomic repository snapshot.

Read-only enumeration uses `collect_lift_metadata` / `select_lift_metadata` and
retains invalid progress as invalid rather than hiding a function. Strict inspection
still rejects malformed progress. Legacy single-function metadata before includes or
split across adjacent comments remains readable; combined files require attached
records. Unscoped tag readers and exact-progress repair reject multiple records,
including incomplete member tags, instead of selecting or overwriting the first.

Reviewed source expectations retain every boundary address: singleton values remain
integers and grouped values are sorted tuples. Scans require complete agreement and
reject duplicate owners; they do not choose the first path. Layout promotion remains
pending. Status cache v7 keys each target/source/address and checks record identity;
grouped native status/cache admission remains gated. Index v16 invalidates older derived
semantics; preserve the old index before rebuilding. A rejected candidate never
replaces it, and an older schema is historical evidence, not current acceptance.

Type-use indexing retains known type spellings from fingerprinted source claims.
Supported attached implementation ranges link to the matching indexed target,
source and address; unsupported metadata and outside-body spellings remain source
context. Separate function keys preserve sibling uses. These lexical candidates
do not prove C type binding, close include consumers or enable type transactions.

Macro association uses authored ranges even when the analyzer omitted a sibling;
that sibling's uses remain unresolved, never borrowed by the sole indexed function.
The macro coverage guard now checks exact source/function identities against
hash-matching source samples and complete authored/indexed membership. Contextual
source and included-header uses require every affected member; direct lexical uses
retain their own range. Strict unchanged-assessment mode still rejects unresolved
identities. This does not migrate type-use identities or enable grouped application.
Native resolve/compare and status preflight
reject grouped files before comparison/cache reuse; macro/type preparation, proposed
C images and filename-changing naming facts retain corresponding guards.

`bin/cc` now passes its one complete maspsx translation through
`build.translation` before assembly. Ordinary sources pass through unchanged;
grouped lifts require attached records, exactly one explicit source owner and
complete map/Splat-resolved assembler membership. CMake tracks the adapter,
partitioner and consumed ownership inputs. This low-level producer retains the
selected GCC and ordered flags; it does not prove a consolidation preserved each
member's PRE profile. [Compiler settings](compiler-variants.md#function-compiler-settings)
resolve attached function tags, then legacy path-keyed
`config/compiler/object-flags.cmake`, then project defaults. Members with different
effective selections reject; no setting is silently discarded. Profile migration
must freeze old paths/settings before moves, require one compatible effective
profile and reproduce it at the new path.

`build.sections` partitions output; `match.placement` serves explicit grouped
comparison, not ordinary production matching. The kernel rejects unsupported text subsections/attributes, unplaced allocated sections,
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
Complete producer/include/toolchain freshness, shared
data, instruction selection, complete member coverage and grouped status/cache admission
remain open; do not bypass existing guards. Current CMake ownership
dependencies are conservative across targets, not a performance optimization.

## Symbol-qualified comparison

`match.extraction` reads a bounded, immutable ELF32 little-endian MIPS byte image
and selects one defined function by name, linked address and executable section.
The complete positive, aligned symbol extent must fit the section. Missing, zero,
ambiguous or unsupported symbols reject; expected length and neighboring symbols
never supply a guessed boundary. Reserved entries, local/global ordering, section
indices and declared alignment must be valid. Canonical PSX linked images can put
extra zero-size local FILE/NOTYPE entries after the declared local cutoff;
only those are allowed before the first nonlocal symbol. Function symbols retain
the declared partition; locals after a nonlocal reject. Overlapping executable ranges reject,
even without allocation flags. The input limit is 64 MiB; extraction also respects
the caller's native output limit without truncating a function.

Ordinary byte comparison and flag search compare that whole function, not a
`.text` prefix. A valid longer or shorter function is `different`, with its actual
size retained. Diagnostics disassemble the actual address range, including extra
instructions. Instruction-only agreement or inconsistent sizes cannot yield native
exactness. Status-cache v6 discards older entries and binds the supplied target
model plus object flags/catalog bytes or absence. Nested metadata and sequence
order survive; dictionary order is immaterial. This does not establish atomic
input capture, complete compiler/runtime closure, a production re-audit or renewed
source attempts.
Status reloads target models after successful or failed batches and rejects changed,
missing or unreadable metadata before comparison or fallback, without caching the
invalid result. This is a post-build boundary check, not continuous native freshness.

Grouped extraction shares this reader while retaining section placement, complete
symbol-inventory and flow checks. The parser inherits the original native deadline,
never a renewed allowance; the tighter inherited cutoff covers parsing, flow
boundaries and return. Retained ELF
replay is compatibility evidence, not fresh compilation or native acceptance.
Public grouped-source guards and the remaining C1.3 producer, profile, cache and
consumer gates remain in place.

## Configured compiler profiles

Before planning a move, inspect two to 32 explicit source files in one target:

```sh
bin/combiner profile emi/etc/game/00 src/bof3/ui/advancePanel_game00.c \
  src/bof3/ui/advancePanelXTo17_game00_8019982C.c \
  src/bof3/ui/advancePanelXTo320_game00_801996FC.c
```

This read-only preflight freezes source paths, member identities, ordered compiler
arguments, selected installed GCC executable hashes and configuration inputs. Each
member records its configured command using the default `build/` output path.
It never compiles, configures CMake, generates a compile database or installs a
compiler. Missing compilers, incompatible profiles, ambiguous path keys, ambient
compiler overrides and malformed or unattached compiler annotations reject. An existing
destination must be a selected member; its independently resolved profile must
equal every retained member profile.

Preservation v6 authenticates retained PRE source text against the captured hashes
before resolving original annotations, including removed files and an overwritten
destination. POST settings resolve from live captured destination text, never from
the retained PRE image. Older preservation records are not upgraded or rebound.
For a new or overwritten destination, pass `--destination-source DRAFT.c` and
`--expected-destination-sha256 SHA` together. Profile v4 embeds that bounded UTF-8
image without writing it: attached records must cover all selected members, ordinary
definition names must match, and the draft's effective metadata/path/default profile
must equal every PRE profile. Template expansion and C semantics still need their
later gates. Without a draft, inspection uses existing destination bytes or absent
path defaults. Inspection writes no configuration or sources.

`--migrate-configuration` plans legacy keys in
`config/compiler/object-flags.cmake`: remove superseded member keys and retain the
common normalized compiler/ordered overrides at the destination. Only selected
members' and destination keys may change; unrelated statements/comments remain
byte-preserved, with a missing separator appended before new assignments if needed.
No owned keys or byte change yields `configuration: null`; metadata alone never
creates legacy overrides. Ambiguous keys and unsupported CMake reject. The plan
embeds before/after text and selected PRE settings.

The reader supports one fixed, reviewed CMake recipe digest and a closed literal
object-configuration grammar. Unknown recipe bytes, including harmless edits,
reject until the recipe, argument parity and affected ownership are reviewed.
Never update that digest mechanically. Included configuration and driver inputs
are separately validated/pinned; this is not an arbitrary CMake interpreter.
Only ASCII assignments/line comments with LF or CRLF are supported. Bracket
comments, value-less assignments and trailing assignment comments reject rather
than silently erase an override. Members require aligned 32-bit MIPS C
boundaries with consistent positive file/virtual extents.

The `fingerprint` binds destination absence/content, any draft, and source-path
inventory. Replay with `--expected-fingerprint PIN` before any edits.
Manifest discovery is shared with the domain loader: every consumed TOML under
`config/targets/` is pinned, including non-`target.toml` names. Parsing and cache keys
use the context's captured bytes, not a second unbound file read. Final checks
retain the original inventory and reject additions, removals or input drift.
Every load's actual read set must equal that inventory, with no duplicates.
Binary/source/header claim validation uses captured bytes and optional absence;
the exact consulted claim set is also required on every load, including cache hits.
Placement bounds derive from sampled binary lengths, not a second live stat.
PRE/POST preservation uses that same set; changed reader-owner bytes require fresh
capture rather than rebinding historical records. Captured claim validation does
not make repository layout, runtime or include inputs globally atomic or complete
the remaining closure work.
Profile inspection and preservation project layout semantics from their captured
Splat text rather than reopening it; final physical checks remain. Other layout
and source consumers are not thereby snapshot-based.
Input hashes and mutation metadata remain stable throughout capture. Directory
observations retain all nine stat fields plus a descriptor-bound fingerprint of
direct names, devices, inodes and nofollow types. Only directory size may differ
when the original location, other eight fields and membership remain equal;
directory allocation size can vary without a namespace edit on FUSE/NTFS.
Regular files and symlinks retain exact metadata checks. Complete enumeration is
bounded to 16,384 entries and 4 MiB per directory, including ancestors above the
repository; errors, overflow and expiry reject. Original snapshots never refresh.
Membership endpoints are not mutation-history proof; existing latched watches and
file-content checks remain required at their owning transaction/producer gates.
`--work-deadline` retains the caller's monotonic cutoff.
This command does not migrate source/configuration or compare POST against PRE;
the separate preservation commands below perform pinned transition checking.

Profiles also bind the [declared program graph](#program-observations), not complete
backend or runtime dependencies.
Configured identity is not a fresh version check. Native fidelity, include/shared
state, producer freshness and complete consumer coverage remain unverified; the
report sets verification flags and `write_authorized` to false. This is a C1.3
prerequisite, not C1.3 completion, C1.4 ranking or production consolidation authority.

### Profile preservation across a move

```sh
bin/combiner preservation capture pre-profile.json post-states.json \
  --expected-profile-fingerprint PRE_SHA --expected-post-sha256 POST_FILE_SHA
bin/combiner preservation verify preservation.json --expected-fingerprint RECORD_SHA
```

Both commands are read-only. Capture re-inspects live PRE and emits the record to
stdout; retain its canonical fingerprint externally before any transaction. POST
JSON maps exactly the destination, every old source, target manifest and Splat path
to `{ "sha256": "…", "mode": 420 }` file states; superseded sources use `null`.
Modes describe observed images, not permission-changing operations. The POST-file
digest binds actual JSON bytes. If profile v4 embeds a draft, the destination POST
hash must match its exact UTF-8 bytes; capture replays that draft against live PRE,
and verification checks live POST against both pins. Profiles and preservation
records share the 4 MiB serialized-document bound; older schemas require fresh
capture, never rehashing or rebinding. A configuration migration requires its exact
POST hash in that same complete source/manifest/layout map, preserving the existing
file mode. PRE text and settings authenticate against PRE pins; only the computed
selected-key edit is permitted. Verification replays PRE profiles with retained
configuration and checks POST profiles against live configuration. Partial or
unrelated configuration changes reject.
These commands do not apply the migration, edit/delete sources, refresh indexes or
build. A deletion-aware combiner apply, owned rollback, native all-member
checks and independent final acceptance remain required before production use.
The shared runtime now supports explicit owned deletions, v4 absent-POST recovery
and identity-backed restoration alongside replacement images; see
[recovery capture](harness.md#recovery-capture). This prerequisite does not create
a combiner application or authorize consolidation. Its domain adapter must bind
the exact deletion set to the reviewed joint source/configuration plan and preserve
all existing execution, audit and acceptance gates.

Verification requires exact pinned POST bytes and unchanged captured inputs, source
and manifest inventories, ordered compiler profiles, selectors and symbol identities.
Only manifest source-list replacement and explicit Splat `@source:` C-boundary
replacement may change semantically; all other manifest/layout fields must remain.
YAML aliases, anchors, tags and non-string or duplicate mapping keys reject.
Event preflight bounds nesting before composition; the validated node graph is
constructed once and must remain JSON-compatible. Python-safe parsing retains the
existing YAML grammar: the C-safe backend has incompatible acceptance behavior.
Byte/depth limits, cooperative deadline checks and parser disposal remain intact.
Ancestor-drift errors report raw identity/stat tuples and membership fingerprints;
the qualified directory-size exception above does not establish a kernel cause.

Profile capture reuses only bytes bound to a fresh `common.inputs.read_input`
metadata/hash sample; final verification still rereads every input. Leaf metadata
is reused within that sample, never between calls. Ancestor symlink checks remain
mandatory even for absent leaves, and outer identity/ancestor/single-link checks
remain separate. The input-reader owner is included in preservation pins.
Layout/PS-X, source metadata/claims, symbol maps and lexical interpretation owners
are pinned with their shared I/O, file/path and deadline helpers. Validation rejects
initial input omissions and requires its complete observed map to equal retained
PRE-plus-POST states, including optional absence and later reads. Missing owner
pins require fresh capture and review, not silent present-state backfill. Explicit
repository-owner pins are not full runtime/import or loaded-code authentication.
The initial retained-input read loop uses its own fresh `InputBatch`, closed before
semantic checks. State/hash/mode and absence comparisons stay exact; every full
verification and final guard remains. Batches are never shared across passes or
output mutations, and no prior validation success substitutes for fresh reads.
Configured flags and compiler IDs use captured override text; selected variants
use a lazily parsed, context-local captured catalog rather than later file reads.
Absent and empty catalogs remain distinct; unused schema validation is not moved
earlier, and missing configured IDs never fall back. Ordered flags and public
file-loader policies stay unchanged. Final physical and complete input-map checks
still apply; parsed variants do not authorize reuse across mutations.
Source-ownership scans use captured C text; compiled-name resolution parses the
captured target-local map once per pass. Trusted reader/map injection preserves
metadata, collision and boundary rules but does not prove freshness. Default
domain file APIs remain unchanged; an empty supplied map never falls back to disk.
Final physical and complete observed-input checks still reject drift or omissions.
Full destination code and metadata review remains mandatory: membership does not prove C semantics.
The repository-bound record requires an independently retained fingerprint; its
self-hash gives no review authority. Success proves configured preservation only,
not native equality, complete toolchain/include identity or producer freshness.
`bin/cc` consumes v6 preservation records through the build-owned driver described
below; older records require fresh capture. Production consolidation remains gated.
Checks fence mutations during each observation; matching SHA/mode endpoints do not
prove inode continuity between PRE and POST. The CLI checks the inherited cutoff
after rendering, before stdout publication; callers still require complete output
and successful terminal exit, not atomic publication if writing itself crosses the cutoff.

### Joint transaction preparation

```sh
bin/combiner transaction prepare pre-profile.json images.json \
  --expected-profile-fingerprint PRE_SHA --expected-images-sha256 IMAGES_FILE_SHA
bin/combiner transaction verify transaction.json --expected-fingerprint TRANSACTION_SHA
```

These commands only prepare/recheck live PRE; neither applies nor accepts edits.
The profile must embed the exact destination draft. `images.json` maps every
destination, superseded source, manifest, Splat and planned configuration path to
`{ "text": "complete UTF-8 file contents", "mode": 420 }`, or `null` for an
explicit deletion. Preserve observed modes for existing files; a new destination
declares its intended mode. The CLI pins the images file's actual bytes.

Preparation checks the complete path/deletion set, compiler preservation, proposed
manifest/source-list and Splat-marker relocation, and exact configuration migration
before any mutation. It retains joint text, PRE/POST states, owner/input pins and
inventories under one externally retained fingerprint. Verification re-derives the
record against live PRE and rejects changed images, owners, inventory or inputs;
it is not the preservation command's live-POST check. Records share the 4 MiB bound
and original `--work-deadline`. Permission changes, source writes, index refreshes
and native commands are not performed. `native_verified`, `coverage_verified`,
`write_authorized` and `accepted` remain false. The separate rehearsal below exercises
temporary application and owned restoration; permanent application, complete consumer
gates and independent production acceptance remain open.

New preparation emits transaction v3, adding `build.programs` and `common.lookups`
to the v2 owners. The history owner validates exact versioned owner sets: v1/v2
retain preservation v5/profile v3; v3 requires preservation v6/profile v4.
Fingerprints, images, modes, ownership and false acceptance flags remain mandatory.
Older transactions are restoration history only: live verification and rehearsal
require fresh v3 preparation, never automatic backfill.
Recovery manifest v1 is unchanged; historical validation grants neither restoration
authority nor current compiler/dependency freshness.

### Transaction rehearsal and recovery

```sh
BOF3_PRESERVATION_RECORD=/absolute/repository/out/group-preservation.json \
BOF3_PRESERVATION_FINGERPRINT=RECORD_SHA \
bin/combiner transaction rehearse transaction.json \
  --expected-fingerprint TRANSACTION_SHA --implementation-run-id RUN_ID \
  --object out/group-check/rehearsal.o \
  --output out/reviews/evidence/rehearsal.json \
  --work-deadline ORIGINAL_MONOTONIC_CUTOFF \
  --cleanup-deadline LATER_MONOTONIC_CUTOFF
```

This explicitly mutating experiment always restores owned PRE; it is not a permanent
apply command. Supply the prepared transaction's exact preservation record through
the existing explicit or source-qualified routing, without changing compiler
overrides. Fresh preparation is required when transaction owners change. Dirty
workspaces require `--adopted-baseline` equal to the current shared workspace digest.
An initialized Git index and existing canonical source/object parents are required.
The report, `OUTPUT.recovery.json` and every native artifact must be absent and
disjoint from retained inputs.

Rehearsal holds one writer lease, rechecks live PRE, and captures shared v4 deletion
recovery before applying exactly the prepared images. Before the first source edit,
`OUTPUT.recovery.json` retains that invocation's recovery path, structured digest and
raw-file SHA, run ID and transaction pin; callback/publication failure prevents edits.
Use this receipt after interruption, never select the newest matching recovery file.
The receipt grants no restoration authority. Rehearsal checks exact POST, invokes
the grouped comparator once, then restores the identity-bound old source, manifest,
Splat and configuration images, including absent destinations. Direct source matching
aids reject before application. No broad workspace or index restoration occurs.
Unrelated drift stops success after safe owned rollback; retained recovery permits
parent inspection, not automatic retries. No new directories under source are created.

One original work cutoff bounds forward work; a separately supplied later cleanup
cutoff fences each rollback transition, including partial application cleanup.
Expired cleanup starts no further owned transition and retains remaining backing;
this is cooperative fencing, not interruption of an in-flight filesystem syscall.
Neither cutoff renews on retry. A child `ProcessCleanupError` leaves evidence and
current owned state untouched
until actual writer-tree termination is established; never roll back beneath an
unconfirmed live child. These safeguards use the common runtime and restoration
owners, not a second recovery engine.

After confirmed restoration, the absent report is exclusively published. Exit zero
means the temporary comparison matched all selected function bytes and PRE was
restored; differences exit one. Native failure, expiry or unverified restoration
raises instead. The report retains historical POST comparison evidence, never current
POST admission: `accepted`, `reusable`, `write_authorized`, `native_verified` and
`coverage_verified` remain false. Full compiler/consumer closure and actual native
validation are separate production gates, not proven by synthetic characterization.

`bin/combiner inspect-recovery RECORD --expected-recovery-digest PIN` reads shared
recovery evidence without changing files. `bin/combiner recover RECORD
--expected-recovery-digest PIN --authorization AUTHORIZATION
--expected-authorization-digest AUTH_PIN` uses the same independently pinned parent
authority, distinct run identities, actual terminal evidence, absent publication and
workspace/index safeguards as other domains. Historical manifest validation does
not demand live PRE, so partial/deleted POST remains inspectable. See
[recovery capture](harness.md#recovery-capture). Standalone recovery remains
parent-supervised; the shared recovery CLI does not enforce the stored cleanup clock.
No new semantic acceptance is granted.

### Grouped comparison

With live POST and its externally retained preservation record, compare the entire
group through one fresh configured producer invocation:

```sh
BOF3_PRESERVATION_RECORD=out/group-preservation.json \
BOF3_PRESERVATION_FINGERPRINT=RECORD_SHA \
bin/combiner compare src/bof3/ui/group.c \
  --output out/group-check/attempt.o --work-deadline MONOTONIC_CUTOFF
```

The output parent must already exist canonically. The object, assembly listing,
producer receipt, linked ELF and linker script must all be absent; no old artifact
is reused or replaced. Existing source-qualified preservation routes also work.
The CLI owns a writer lease; `comparison.compare_members` requires that same-process
lease when called by another owner. No child borrows it. Grouped compiler selection
honors attached metadata and legacy object settings, not just an ambient default.

The producer returns its fresh in-process receipt pin; placement returns hashes of
the private linked/script images before publication. Later reads must match those
pins. Input/output watches, final hashes, writer checks and one original deadline
cover compilation, placement and every member. Captured inputs use a 64 MiB per-file
cap, not a bound on unobserved backend reads; `--output-limit` defaults to 2 MiB and
cannot exceed 64 MiB. Captured inputs and outputs must be single-link files; final
descriptor-bound identity and mutation metadata detect restored-byte writes too.
Shared `InputBatch` supports opt-in bounds and metadata without changing defaults.
`build.dependencies.IncludeSnapshot` captures literal header bytes and every
preceding absent lookup candidate through confined single-link reads. Comparison
v3 reports that exact `include_inputs` set, protects it from output collisions,
and validates samples after watch registration and before publication. Immediate
directory mutation watches and candidate-path watches latch transient shadows;
stage guards drain events without repeatedly hashing all headers. Full endpoint
checks still reread every captured input. Limits are 16,384 lookup paths, 64 MiB
per file and 128 MiB total captured content. Existing discovery callers are unchanged.
All syntactic conditional branches are conservatively traversed, not evaluated.
Missing headers, absolute/noncanonical names, macro operands, include-next/import,
pragmas and alternative preprocessing spellings reject; no system-header fallback
is inferred. Strict scanning accepts LF/CRLF, rejects bare CR and splices only once.
Source/authored-header policy checks consume the captured bytes.
This closes observed literal-search gaps, not complete compiler/preprocessor,
flag-induced include, toolchain/runtime or producer-receipt dependency closure.
[GCC 2.7.2](https://ftp.gnu.org/old-gnu/gcc/gcc-2.7.2.tar.gz) also consults
`header.gcc` mappings in each searched directory and the
candidate header's parent. Literal capture records their absence before checking
the candidate; an existing mapping rejects, even if empty. Later creation or
creation/removal is watched, including at earlier roots with no selected header.
Unsearched roots are not traversed. This rejects unsupported remapping; it does
not implement mappings or prove native closure. These paths share existing bounds.

The shared `SourceImage` binds canonical nominal path, immutable bounded bytes,
kind (`current`, `retained` or `prospective`) and expected SHA-256. Profile replay
passes the original PRE digest; prospective text is prediction, not acceptance.
Current images must equal captured physical bytes. Virtual images never reopen
their seed path: dependency lookup uses the nominal parent, while `content` and
`include_inputs` describe physical reads only. An explicit include of the seed
path reads that physical file, not the virtual image again. Comparison v4 records
`source_image` separately; the retired combiner module has no shim. This interface
does not prove virtual-image native replay or add compiler flag, spec, default-search
or runtime dependency resolution.
Fast snapshot checks enforce the inherited work cutoff even with no physical
inputs and latch expiry as failure; closing remains available after expiry.

Membership comes from preserved selectors, target-local symbols and unique reviewed
C boundaries. Captured manifest/layout/maps/binary bytes supply bindings and original
ranges; SDK/shared maps never establish member identity. Binding precedence remains
shared, SDK, local by address, with surviving duplicate/case-colliding names rejected.
PS-X `t_addr`/`t_size`, payload offset and file/virtual coordinates must agree. Every
range must be finite, aligned and at least two instructions; no return-based size guess.

Comparison links the complete group once and checks full symbol extents. Different
sizes stay different, never prefix matches; equal-sized functions also pass the
existing flow checks. Reports include every member's sizes, hashes and first differing
byte. Exit zero means all selected function bytes match; a reported difference exits
one. Unsupported placement/flow or stale inputs fail rather than emit exactness.
Unplaced allocated data rejects: this route does not invent shared-data ownership.
Failed native work retains artifacts for inspection; unconfirmed process cleanup is
not retry or restoration authority.

This is generated-artifact work, not source application or final acceptance. Reports
set `reusable`, `accepted`, `write_authorized`, full-input closure and consumer coverage
false. Full include/toolchain/runtime closure, initialized/shared-data proof, ordinary
matching/status consumers, transactions/recovery and production acceptance remain open.
Intercepted fixtures cannot establish native equivalence; actual compiler and all-member
positive/negative byte checks remain required for production admission.

### Compiler dispatch

`bin/cc` is a thin bootstrap; `harness.build.driver` executes GCC → maspsx → assembler
from the immutable `build.invocation` recipe. `build.arguments` owns operand
classification; `harness.build.dispatch` binds the invocation and
`harness.build.preservation` verifies frozen history. Combiner retains capture policy;
there are no compatibility re-exports from its former verification owner.
Dispatch pins the caller's actual working-directory path and filesystem identity,
watches its linkage, and classifies relative operands against that captured base.
Every GCC, maspsx, assembler or explicit-driver stage receives the same directory;
observed context drift rejects rather than silently switching to repository root.
Dispatch also fingerprints the complete incoming environment from one captured
mapping, without adding unlisted names or values to the receipt. Validation checks
that digest before and after input verification; absent and empty values differ.
This detects endpoint drift, not mutations reverted between checks. The captured
recipe binds ordered stages, typed temporary operands and derived environment
fingerprints; rendering never substitutes substrings in literal options. Relative
compiler PATH entries resolve from the captured working directory. The driver
consumes that recipe rather than reselecting stage controls from ambient state.

Producer v5 records `build.execution` v2 edges: successful child commands/environment
digests, stdin/generated-image hashes, partitioning and final publication. Evidence
returns directly from the driver after cleanup and terminal checks. Invocation,
recipe and execution fingerprints stay distinct; comparison v5 binds the producer's
execution fingerprint. Verification renders the recorded temporary binding without
allocating or rereading deleted staging. Unknown environment values are not printed.
Older receipts cannot acquire these bindings retroactively. New compilation requires
profile v4/preservation v6 and current owner pins. Older retained transitions keep
their strict historical-restoration validation, not current admission.
Recorded execution is not authenticated native proof: endpoint samples do not
detect reverted mutations or establish complete compiler search/import/runtime
dependencies. Reuse, native equality and source-adoption authority remain denied.

### Program observations

`build.programs` derives ordered executable candidates, explicit script operands
and the supervisor executable from each actual `build.invocation` stage. Bare
programs retain every effective PATH candidate, including absent earlier entries;
empty PATH components mean CWD and an absent PATH uses the platform default.
This is candidate evidence, not proof of the kernel's selected executable. Existing
compiler/direct-driver argv canonicalization is unchanged; raw lookup spelling,
alias edges and physical nodes remain separate. The captured supervisor tuple is
also passed to the common process launcher without changing its cleanup protocol.

`common.lookups` traverses only declared names and encountered alias targets through
held nofollow directory descriptors. Single-link regular files bind bytes and
mutation identity; missing entries and non-directory blockers remain explicit.
Repository and external-host roots have distinct observational types; the external
`/` anchor grants no enumeration, arbitrary sibling reads or write/restore authority.
Retained validation replays bounded lookup semantics and rejects unrelated nodes
or inconsistent trails, terminals and root bindings without live filesystem reads.

Limits per capture pass are 2,048 candidates, 8,192 nodes, 512 directory descriptors,
256 steps and 40 aliases per lookup, 16 KiB spellings/targets, 64 MiB per file,
256 MiB total bytes and 512 KiB serialized graph. A concurrent watch has additional
descriptors. Discovery is followed by watch installation and a complete recapture;
later checks bracket fresh reads with latched events. Failure/expiry cannot revive
a snapshot; closure remains available. Output, publication scratch/quarantine and
staging-family collisions reject before their creation. No successful prior check
substitutes for a fresh validation.

Profiles omit incidental source/output operands from this common program graph;
concrete recipes bind those separately. Dispatch retains the graph in its invocation
fingerprint, transitively binding execution and producer/comparison receipts.
Profile v4 and preservation v6 require it; historical schemas are never backfilled.
This direct-stage graph still excludes GCC backend/spec searches, imported modules,
Python startup, shell interpreters and ELF loaders/libraries. `complete:false`,
configured provenance and non-reuse remain mandatory; native fidelity is unverified.

For an explicitly admitted grouped-unit compile, supply
`BOF3_PRESERVATION_RECORD` and `BOF3_PRESERVATION_FINGERPRINT` together. The record
must select the actual canonical `src/bof3/` source and configured GCC executable.
Arguments must exactly reproduce the ordered preserved driver flags followed by
`-c ABSOLUTE_SOURCE -o ABSOLUTE_OBJECT`; grouped objects belong under `build/` or
`out/`. Explicit configured `PSX_GCC` is permitted only when it equals that selected
compiler; other unsupported compiler environment overrides still reject. Profiles
reject any set `C_INCLUDE_PATH`, `CPLUS_INCLUDE_PATH` or `OBJC_INCLUDE_PATH`, including
empty values: historical GCC can interpret an empty language path as CWD, unlike an
unset variable. Empty `CPATH` and empty executable-override fallbacks remain allowed;
no environment is silently sanitized. This guard is not complete dependency capture.
The bootstrap is closed by a reviewed digest, not guessed shell-default parsing.

For mixed ordinary/grouped or multiple grouped units, instead supply
`BOF3_PRESERVATION_ROUTES=/absolute/repository/out/routes.json` and
`BOF3_PRESERVATION_ROUTES_SHA256=<external raw-file SHA-256>`. Both variables are
required; combining them with the explicit pair rejects. The closed JSON shape is:

```json
{
  "schema": "bof3.preservation-routes/v1",
  "root": "/absolute/repository",
  "records": {
    "src/bof3/ui/panel.c": {
      "record": "out/panel-preservation.json",
      "fingerprint": "<external preservation fingerprint>"
    }
  }
}
```

The table contains 1–4096 canonical source keys and distinct repository-relative
record paths. Duplicate keys, malformed pins, path aliases, conflicting inputs,
missing grouped entries or a recorded source becoming ordinary reject. Routed
invocations require one canonical `src/` C source. Ordinary sources absent from
the table keep ordinary dispatch; each selected group still needs its own valid
PRE/POST record and exact compiler/arguments. Unselected records receive schema
and path checks, not preservation acceptance. The table is a selector, not an
authority store: retain its raw SHA independently. Its bytes, identity, ancestors
and caller selection remain watched through dispatch and receipt validation;
no process-global environment mutation or automatic record discovery occurs.
Routes reserve the table and every named source/record, even for ordinary calls.
Direct and routed grouped dispatch also protect every selected preserved input
against primary, diagnostic and receipt outputs, including resolved aliases.
V3 records also bind global source and manifest state: another consolidation can
stale an earlier record. Routing does not compose or rebase those histories;
multi-group build acceptance still requires mutually valid preservation evidence.

Detected grouped inputs reject before native launch without this external pin, even
on delegated-driver, preprocessing or assembly-only paths. Opaque response files,
stdin inputs and unsupported grouped forms reject rather than bypass inspection.
Ordinary named single-function and assembly pipeline arguments retain GCC selection,
maspsx version/division controls and assembler flags; link mode remains unsupported.
CLI translation alone cannot admit a grouped unit without its active invocation.

The in-process invocation binding is retained across compiler, translation and
assembler checks, including source grouping, profile, argument, compiler/environment
identity and input observations. Native children use the common supervised runner;
`BOF3_WORK_DEADLINE`, when present, is an absolute monotonic cutoff and never renews
the inherited budget. Inputs are rechecked between diagnostic/object publication,
after publication/stdout, and after cleanup before success. Resolved compiler or
delegated-executable bytes are bound even for ordinary sources. `common.observation`
owns descriptor-bound Linux path watches: input-entry changes and ancestor moves
remain latched, including create/delete cycles of absent inputs or missing parent
components. Ongoing validation ignores unrelated sibling names; owned outputs cannot
mask input changes. Installation conservatively rejects concurrent directory activity,
even from unrelated siblings. Setup races, malformed/overflowed queues and lost watches fail
closed. Event tracking supplements content and identity checks, not full include
or toolchain provenance.
Object and diagnostic
assembly outputs are staged privately, then use the existing owned publication
backend; this is not atomic publication of a multi-file bundle. Validation failure
before publication preserves existing outputs. Unconfirmed child cleanup retains
staging instead of racing live descendants; publication/durability failures require
inspection of retained output/quarantine state, not blind rollback. Publication that
crosses the cutoff fails with retained recovery information; the existing cooperative
backend cannot promise an atomic time fence or rollback after publication.
Compiler pipe failures remain nonzero instead of inheriting the shared CLI's pipe-success
policy; failed-stage exit status is preserved when its diagnostics encounter a closed pipe.

Grouped CMake lift targets always run the combined producer, including unchanged or
future-dated objects; receipts never authorize reuse. Aggregate targets depend on
unique lift owners. `build.inventory` captures source classification through the
same metadata parser as dispatch; a pinned, always-run gate rejects changed classes,
membership or classifier owners even when source timestamps were restored.
Inventory v4 binds each source's effective compiler settings, every discovered TOML
path and raw hash, plus object flags and the variant catalog as bounded hashes or
verified absence.
Empty files are not absence; aliases, read failures and in-capture changes reject.
V1–v3 snapshots require reconfiguration. CMake watches all TOML names and contents,
captures inventory before including flags or reading target roots/claims and
checks it after constructing targets. Defaults and configuration syntax remain
unchanged. These endpoints do not prove configure-time ABA continuity or an atomic
snapshot across CMake and its helpers. Ordinary
unchanged objects remain timestamp-driven. The public frontend regenerates stale
inventory; raw CMake/Ninja users must reconfigure on its diagnostic.

Supported graph entrypoints are friendly lift, target-group and `lifts` targets,
plus Ninja's direct object outputs. Make's internal `build.make` fragments are not
entrypoints or proof. A single ambient preservation pair covers only its admitted
group and rejects rebuilding ordinary units. Use the source-bound table for mixed
or multi-group routing; graph wiring alone does not prove native aggregate success.
An active parent writer lease cannot be borrowed by a child producer. Compiler
configuration migration, full include/toolchain freshness, native equality and
remaining transaction/cache consumers stay open. Language-free generator fixtures
with literal Python producers do not prove a native project build or consolidation.

`build-adapter producer -- <compiler arguments>` always compiles a guarded grouped
unit under the repository writer lease, then writes `<object>.producer.json` with
configured invocation provenance, execution edges and object/diagnostic hashes. Replaced receipts
remain as recovery material; artifacts cannot overwrite preserved inputs.
Compilation starts with an empty, invalid receipt;
failed final checks attempt to invalidate new publication. Unconfirmed invalidation
is an error requiring recovery inspection. `build-adapter receipt --expected-sha256
<external receipt pin> -- <compiler arguments>` freshly verifies the preservation
record, configured inputs, recipe, execution edges and outputs. Missing, reordered,
failed or inconsistent execution records reject, as do changed publication hashes.
Both commands require either
the explicit preservation pair or the source-bound routing pair. Receipts expressly
deny reuse authority: complete input
closure, native consumer integration and byte equality remain open. A successful
configured-provenance check never permits skipping compilation or proves a prior
invocation finished within its deadline; retain terminal status separately.

## Planned discovery and ranking

The cleanup node is **combiner**, with explicit discovery, assessment, audit and
transaction modes under one concern skill. Beyond inspection/profile preflight,
these modes are queued, not implemented. Candidates require two distinct functions and
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
