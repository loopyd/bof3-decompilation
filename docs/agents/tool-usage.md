# Tool usage

Work on one `TARGET@0xADDRESS` at a time. Commands keep terminal output short;
full generated evidence is written under `out/`.

## Output budget

Context-heavy commands accept `--detail minimal|normal|full`:

| Level | Use | Output |
| --- | --- | --- |
| `minimal` | candidate scouting, small models | decision fields or one summary line |
| `normal` | daily iteration | labeled metrics or the first bounded diff hunk |
| `full` | debugging/tool development | complete rows, function records, or diff |

`normal` is the text default. Plain `--json` remains full for automation.
`-o FILE` always writes the complete artifact. For payload commands such as
`m2c`, use `-o` and open only the file you need to edit.

## Documentation operations

Use `$bof3-docs` for Markdown reference inspection, search, editing, repair or explicit
one-document compaction with the native read, search and edit tools — the harness exposes
no Markdown transport, so link and anchor hygiene is the skill's own responsibility.
See [documentation.md](documentation.md) for scope and preservation contracts.

## Area text

`bin/harness text` wraps the project Rust tool `tools/rust/bof3-text`. It covers the **text**
subfiles of an EMI archive: `dialogue` (load argument `0x80010000`, one per US area archive) and
`battle` (two-bank system/battle text, `0x8001A000`), plus `heuristic` raw-text instances the
scanner latches inside other subfiles — a lead, never verified text. It also builds a retained
**corpus index** so a search covers every archive at once. The layout, character table and
command bytecodes are owned by [the dialogue text spec](../specs/formats/dialogue-text.md).

```sh
bin/harness text index    ARCHIVE.EMI [--class CLASS] [--entry N]
bin/harness text scan     ARCHIVE.EMI [--min-run N] [--class CLASS]
bin/harness text extract  ARCHIVE.EMI -o OUT.json [--entry N] [--class CLASS] [--latch ENTRY:OFFSET+LENGTH]
bin/harness text validate OUT.json [--commands]
bin/harness text query    ARCHIVE.EMI [--entry N] [--grep TEXT] [-i] [--commands] [--class CLASS]
bin/harness text pack     --original ARCHIVE.EMI --text OUT.json -o OUT.EMI [--entry N] [--class CLASS]
bin/harness text build-index                # retained corpus index (rebuilds only when stale)
bin/harness text search   --grep TEXT [--class CLASS] [--match-offset] [--windows PATH] [--payloads PATH]
bin/harness text probe    ARCHIVE.EMI [--round-trip]   # structural evidence, class-agnostic
bin/harness text map      [--out PATH]                 # read-only text-versus-data report
bin/harness text verify                                # corpus round-trip gate
bin/harness text windows                               # classified-window registry the latching modes honour
bin/harness text payloads --root out/extracted/BIN     # the payloads the inventory identifies (audio, music)
```

`bin/harness text windows` derives the classified-window registry from the reviewed
`textbin` ranges each target record declares, in `tools/rust/bof3-text/src/cli.rs`. It reads a
target's identity from the **first** `disc_id` and `load_address` in that target's
`target.toml`, so a multi-member manifest must declare its own archive first and list members
after it. The builder also requires the named reviewed ranges (`ETC/COMMU00.EMI#0`,
`SCENARIO/SCENA00.EMI#0`, `WORLD00/AREA026.EMI#13`) to be present, and fails rather than
quietly producing a smaller registry.

Documents are **JSON only**; there is no second codec. `-o` always names a separate output, and
outputs are **create-new**: an existing destination is refused, so `pack` can never replace its
source. Dialogue controls round-trip as `{command(arg, arg)}`; every byte with no known meaning is
preserved as `{byte(0xNN)}` rather than guessed. A row's bytes are its **extent**
(`[offsets[i], offsets[i+1])`), which also holds a choice row's option list, so an edit must fit
that extent — overflow is refused, never relocated. All **244** US text subfiles (200 `dialogue`
and 44 two-bank `battle`) parse and round-trip byte-identically, and `probe` confirms that no
subfile outside those two classes has the banked structure. Token meanings are confirmed against
the US renderer and the reference translation tool.

`--grep` matches a **command-transparent** reading: control commands and their operands are
transparent, so a phrase broken by a command is found while raw command bytes are never
matchable. Rebuild with `just setup`, or
`CARGO_TARGET_DIR=build/tools/rust/bof3-text cargo build --release --manifest-path tools/rust/bof3-text/Cargo.toml`;
`bin/harness text` also builds on demand, and `PSX_BOF3_TEXT` overrides the binary path. Evidence
is indexed in `out/text-format/README.md`.

## Cleanup opportunity routing

Use [`bof3-lift-loop`](../../.agents/skills/bof3-lift-loop/SKILL.md) for bounded
active-session worker/reviewer missions. The harness does not launch Codex or model
processes. `bin/harness decomp diagnose` and `audit` remain deterministic local tools;
their [mission contract](codex.md#parent-lift-diagnosis) preserves evidence and
independent acceptance. Transport success never authorizes a transaction. Type,
macro, naming and combiner owners retain their [rollback and recovery gates](harness.md#policy-versus-mechanism).
Check-only revalidation grants no source-restoration authority.

```sh
bin/harness agent context cleanup macro-opportunity TARGET ID
bin/harness agent context cleanup combiner-opportunity TARGET ID
bin/harness agent context cleanup type-opportunity TARGET ID
bin/harness agent context cleanup naming-opportunity TARGET ID
```

Each read-only prefill retains one opaque owner ID and selects its macro, type, naming or
combiner skill. Routing checks neither candidate freshness nor target membership;
the owner must verify both before work. It grants no transaction authority and
never replaces a frozen lead or rebuilds an index. See
[harness routing](harness.md#policy-versus-mechanism) and the
[macro specification](macros.md).
The older `type TARGET OLD -> NEW` form still means identity maintenance.

## Naming facts and checks

```sh
bin/harness naming list TARGET --kind debt --limit 20 --json
bin/harness naming list TARGET --kind identifiers --json
bin/harness naming check TARGET TARGET@0xADDRESS --json
```

Use `symbols`, `references`, `files` or `all` for other factual views. Lists do
not build or refresh indexes; a limit bounds display without reducing findings.
Debt categories and lexical references are leads, not semantic or storage proof.

Check validates maps, bindings, reviewed identities and live native consumers.
Selectors expand to complete source groups and current callers. Possible
cross-target consumers are reported as unavailable until checked explicitly under
their own targets. Omit selectors to check the selected target. Inspect `scope`,
`consumers`, findings and incomplete coverage in JSON; text presents the same facts.
Exit 0 means complete passing coverage, 1 means findings/unavailable evidence and
2 means an invalid request. Checks never edit reviewed files or approve names.

The [naming skill](../../.agents/skills/bof3-naming/SKILL.md) owns investigation,
direct edits, independent review and scoped recovery. Use existing change/campaign
notes for unfinished debt; retain historical reports as evidence. Metadata belongs
to `bin/harness source metadata list/check` and its skill branch. A current exact
record requires the live full-range byte gate before editing and a final recheck.

## Type facts and checks

```sh
bin/harness types list TARGET --kind declarations --limit 20 --json
bin/harness types list TARGET --kind uses --json
bin/harness types list TARGET --kind storage --json
bin/harness types check TARGET TARGET@0xADDRESS --json
```

Declarations include identity, fields, sizes, constraints and conflicts. Uses
retain function/source ownership. Storage rows remain original-access leads;
width agreement cannot prove extent, layout, ABI or semantic role. Use `all`
for the combined factual view. A fresh existing index is required; listing and
checking never refresh it or edit reviewed files.

Checks report unsupported declarations, unknown required layouts, conflicting
representation facts and current native measurements. Selected functions expand
to indexed consumers of their types and complete grouped source membership.
Shared declarations identify other targets requiring explicit checks. Inspect
scope, consumers and unavailable evidence; exit statuses are 0 for complete
passing checks, 1 for findings/unavailable evidence and 2 for invalid requests.
Text and JSON retain the same coverage. Limits affect list display only.

The [type skill](../../.agents/skills/bof3-types/SKILL.md) owns direct refinements,
shared promotion, semantic review and scoped recovery. Exact native results alone
do not prove a representation. Mixed reconstruction preserves exacts, unselected
partials and the independently reviewed expected result for selected partials.

## Ordered workflow

### 1. Prepare the repository

```sh
just setup
just doctor
```

First-time setup requires the host tools listed in the
[README prerequisites](../../README.md#prerequisites). `setup` validates a complete
CUE/BIN set under `inputs/external/`, or extracts
`inputs/external/BreathOfFireIIIv1.1.7z` into the private-assets cache. It stages
the pinned toolchains, builds local tools, extracts reviewed target images and
validates the result. `doctor` verifies the existing installation without rebuilding
or downloading. Run `bin/harness source symbols check` separately for symbol maps.

Use component setup when only one tool needs rebuilding:

```sh
bin/harness setup --component psyqo
bin/harness doctor --component psyqo
bin/harness setup --component rz-psxdec
bin/harness doctor --component rz-psxdec
bin/harness setup --component pcsx-redux
```

The [toolchain guide](../../toolchains/README.md) covers PSYQo/Nugget and the
GCC 2.7.2-PSX plus maspsx reconstruction profile. The
[native decompiler guide](../../tools/retdec-psx/README.md) covers the shared
build directory, idle-CPU job selection and build receipts. The
[emulator guide](pcsx-redux-agentic.md) covers embedded missions and skill links.
Use `setup --help` and `doctor --help` to discover supported component selections;
`just doctor` runs the complete check.

Use `bin/harness media disc` to inspect original disc media, `bin/harness emi archive` to list or
extract an EMI archive, and `bin/harness media str inspect|validate|convert` for STR
media. These are acquisition tools, not function-analysis inputs.

### 2. Bootstrap one new EMI target

```sh
bin/harness emi target BIN/BATTLE/BATL_END.EMI#0
bin/harness emi target BIN/BATTLE/BATL_END.EMI#0 --apply
bin/harness source symbols check
bin/harness source splat TARGET
```

`emi-target` previews before `--apply`; it refuses an existing target and
creates a new identity plus bin-only reviewed layout. `symbols` validates
target-local maps. `splat` regenerates assembly and linker inputs for new or
existing reviewed targets; add `--verbose` only for complete Splat diagnostics.
The harness applies a temporary options overlay under `out/splat/<target>/`:
`create_c_files: false` prevents placeholder C/`INCLUDE_ASM` sources, while
`disassemble_all: true` retains individual assembly output. Reviewed target YAML
and authored C stay unchanged; the overlay is removed after the subprocess exits.

### 3. Build analysis evidence

```sh
bin/harness analysis rz-project analyze TARGET
bin/harness analysis rz-project status TARGET
just index
```

`rz-project` keeps each independently loaded image isolated. Analyze every
stale or missing target snapshot before `just index`; indexing fails unless all
manifest snapshots are fresh, then atomically rebuilds the cross-target cache.
Use `bin/harness analysis rz-project open TARGET` only for interactive investigation.

PsyQ SDK evidence and application:

`bin/harness psyq` owns the permanent PsyQ object-signature evidence family
(`{scan|calls|proposal|import}`); symbol-map mutation remains under
`bin/harness source symbols`.

```sh
# Gather provenance (signatures identify objects; official headers own declarations)
bin/harness psyq import --example
bin/harness psyq scan --all
bin/harness psyq calls --all
bin/harness psyq proposal --all

# Apply reviewed provenance, then regenerate and audit bindings
bin/harness source symbols import-psyq out/psyq/proposal.json --all-qualified --write
bin/harness source symbols psyq-bindings --write          # regenerate manifest-owned psyq_source bindings
bin/harness source symbols psyq-report TARGET             # which SDK symbols the code references
```

The SDK maps live in `config/sdk/psyq-{slus,logo}.txt`; a target selects its
space via the manifest `[psyq] space` key (default `slus`). `import-psyq` writes
reviewed exact candidates into the space's SDK map; `psyq-bindings` regenerates
the compiled manifest-owned `psyq_source` (e.g. `src/bof3/support/slus_psyq.c`)
from it. Nothing edits maps without `--write`.

### 3b. Target analysis (freshness → rebuild → query)

Run the existing commands explicitly, in order:

```sh
bin/harness analysis rz-project status TARGET
bin/harness analysis index
bin/harness analysis query quick-wins --target TARGET
# Or: bin/harness analysis query metrics TARGET@0xADDRESS --detail normal
```

`bin/harness analysis index` reanalyzes every stale snapshot before it rebuilds, so
the status call is a readiness check rather than a repair step. Stop if a target
remains stale after indexing; `bin/harness analysis query` refuses stale
snapshot/index evidence. Use
`--exclusions` to inspect rows rejected by canonical-code checks; use
`--detail full` for complete rows.

### 4. Select one function

```sh
bin/harness analysis query quick-wins --unlifted --detail minimal --limit 5
bin/harness analysis query leafs --unlifted --detail minimal --limit 5
bin/harness analysis query duplicates --unlifted --detail normal --limit 5
bin/harness analysis query metrics TARGET@0xADDRESS --detail normal
bin/harness analysis query quick-wins --exclusions --detail full --limit 0
```

Use `--exclusions` on any ranking command to inspect target-qualified rows
rejected by canonical code checks; it reports `candidate_exclusion` instead of
ranked candidates. Use `quick-wins` for low effort, `hotspots` for caller impact, `leafs` for
bounded call dependencies, `pareto` for visible effort/value trade-offs, and
`duplicates` for exact-byte leverage. Rankings are hints, not promotion proof.
Each ranking call loads target context once, reusing its validated manifest,
original bytes, reviewed boundaries and SDK exclusions across candidates. Context
is discarded between calls; standalone candidate checks load fresh context.
Local symbol-map agreement and index freshness checks remain required. These
read-only observations are not an atomic snapshot or live source acceptance.

Supporting queries:

```sh
bin/harness analysis query calls TARGET@0xADDRESS
bin/harness analysis query xrefs TARGET@0xADDRESS
bin/harness analysis query owners TARGET@0xADDRESS
bin/harness analysis query describe TARGET@0xADDRESS
bin/harness analysis query transaction-scope TARGET SYMBOL
bin/harness analysis query inventory TARGET
bin/harness analysis query symbols NAME
bin/harness analysis query variables NAME
bin/harness analysis query types [NAME] [--target TARGET] [--untyped] [--detail full]
bin/harness analysis query type-uses [NAME] [--target TARGET]
bin/harness types list TARGET --kind storage --json
bin/harness analysis query status
```

`xrefs` matches exact target-qualified destinations, not possible indexed bases.
Data extraction recognizes direct LUI/%lo uses within 12 instructions; it does not
propagate materialized pointers or resolve dynamic indexing. An `address` row is
not a load/store, and empty results cannot prove no accesses or complete writer
coverage. The reviewed table-consumer decoder separately proves its fixed original
loads, not all consumers. Allowlisted structural `exhausted` capabilities are not
independent semantic ladder exhaustion or current no-op acceptance.

`owners` combines reviewed Splat ranges, analyzer ranges, and exact mapped-entry leads; provenance and confidence are hypotheses, never ownership authority. `xrefs` includes call references and decoded data accesses with source address, access kind, and opcode. `describe` reports canonical payload/file offsets, reviewed Splat boundary, exact symbol, and references. `types` inventories target-owned declarations plus the explicitly shared base scalar aliases; full detail includes fields, layout constraints, conflicts, diagnostics, and provenance. `type-uses` reports declaration/use relationships. `types list --kind storage` reports conservative representation and semantic leads only: every inferred aggregate, field, array, prototype, or class-like receiver/dispatch row remains blocked until its independent evidence gaps are closed.

Macro fact listings and native consumer checks
are documented only in [macros.md](macros.md).

Type storage/layout inference requires a decoded load/store with a recognized
width and matching opcode/access kind. Address materialization (`addiu`/`ori`)
remains in reference queries, but cannot establish storage widths, layout fields,
regions or array strides. Aggregate/array `end` is the exclusive end of the
observed access span, including access widths—not a proven object/array extent.
Prototype and blocked semantic receiver/dispatch leads remain separate.
`type-uses` also reports known type spellings from fingerprinted claimed C sources
as `lexical` rows, with occurrence positions and unresolved binding evidence.
Supported attached ranges link only to matching indexed target/source/address
identities; other occurrences retain conservative source context. Comments and
literals are excluded. Same-spelled variables, fields or macros can still be
lexical candidates: these rows never satisfy `types --untyped`, prove a type
binding, close include consumers or grant transaction/native acceptance.
Older index schemas are rejected; refresh explicitly with
`bin/harness analysis index`, which reanalyzes stale snapshots before rebuilding.
Queries never rebuild it implicitly.
Both full and status readers validate the exact generated index definitions on
required tables, including partial uniqueness predicates. Missing, altered or
extra indexes on those tables require rebuilding; unrelated extra tables remain
permitted. This structural check does not authenticate arbitrary row contents.
Read-only manifest loading accepts stable in-repository file-link chains (at most
40 hops), preserving lexical manifest identity and content fingerprints. Directory
aliases, escaping targets, cycles, drift and special files are rejected. Native
profile and transaction capture remain strict: query support does not authorize
linked inputs in those gates.
`RepositoryLayout` reuses held file captures for manifest validation, Splat models
and its raw-byte/digest map; Splat text retains universal-newline normalization.
Full/status recipe validation reuses captured binaries, maps, overlays and layouts;
optional shared maps and overlays retain explicit absence, and unknown reads fail.
Preloaded manifests must agree with the captured generation. Index construction
uses that generation for function, symbol, type and macro facts and fingerprints,
including its fixed shared-helper inventory. Lexical type-use scans retain their
4 MiB byte limit and strict source-alias rejection. Publication remains atomic;
per-input consistency does not imply a simultaneous repository snapshot.
The reverse index fingerprints all source inputs, including legacy snapshot
fallbacks when a target has no explicit source claims. Fallbacks must name regular
`.c` files under `src/`, without source aliases, in an identity-checked snapshot.
Canonical snapshot encoding is shared by capture and lookup; old indexes rebuild.
Before atomic replacement, candidate publication uses the canonical schema checks
and the status validator against the same captured generation, without recapture.
Fresh build connections create the schema in one explicit transaction, enabling
foreign keys before `BEGIN`; journals and synchronous settings remain unchanged.
Atomic schema creation rejects pending transactions and rolls back partial DDL
on failure. The default schema API retains its existing transaction behavior.

`bin/harness analysis readiness [TARGET]` reports snapshot/index freshness and
current naming, type and macro inventory counts using `bof3.analysis-readiness/v3`.
`TARGET` restricts counts to that target; shared macro definitions remain visible.
Use `--detail full` for naming-debt rows. `bin/harness analysis index` regenerates
stale snapshots before rebuilding, so the index never stays unavailable pending a
flag; readiness reports staleness but does not refresh it.

### 5. Lift and iterate

```sh
bin/harness lift m2ctx TARGET@0xADDRESS
bin/harness lift m2c TARGET@0xADDRESS -o out/candidate.c
# edit the metadata-resolved lift source and adjacent target evidence
bin/harness lift asm-diff TARGET@0xADDRESS --detail normal
bin/harness lift byte-match TARGET@0xADDRESS
```

`m2ctx` materializes target-owned declarations. `m2c` creates a complete seed,
not reviewed C. `asm-diff` prints a bounded diagnostic and keeps the full patch
under `out/asm-diff/`; `byte-match` is the acceptance check. For a function with
an out-of-image companion call, run `bin/harness lift companion-check TARGET@0xADDRESS` first;
it exits nonzero until static-call identity, companion boundary/map, reviewed ABI,
and matching caller declaration evidence are all present.

When readable semantics are credible but code shape differs:

```sh
bin/harness lift flag-search TARGET@0xADDRESS
bin/harness lift permute TARGET@0xADDRESS --quiet -j N
```

`bin/harness analysis scan [TARGET...]` lists unlabeled in-image data regions referenced
by lifted functions (BSS globals vs file-backed tables/strings, with reference
counts) — the data-labeling and table-extraction backlog. Each region links any
indexed type candidates with their evidence class, status, width, and blocker;
this linkage is triage evidence, never a promoted layout. `--all` widens to
unlifted functions, `--json` for tooling.

`flag-search` suggests compiler flags from known profiles. `permute` searches
source shapes in a disposable workspace. `promote` validates the canonical
source but never edits source, maps, or layouts.

### Share a decomp.me scratch

```sh
bin/harness analysis scratchpad preview TARGET@0xADDRESS
bin/harness analysis scratchpad share TARGET@0xADDRESS
```

`preview` is local-only and prints the exact payload. `share` creates a public,
unclaimed decomp.me PS1 scratch with the target assembly, the authored C body,
and minimal generated target declarations/context; it prints the resulting URL.
Sharing is opt-in and must never include user media, private assets,
credentials, or unreviewed `out/` candidates. It accepts only a reviewed Splat
function boundary with an authored source file; missing lifting ABI/call evidence
does not itself make that function unshareable. It fails closed for a lift that
uses ignored PsyQ declarations; add a reviewed public declaration boundary
before sharing that class of function. It defaults to the canonical local
`gcc-2.7.2-psx`, mapped to decomp.me's `gcc2.7.2-psx` compiler ID; it does not
change a local compiler/object selection or constitute matching evidence.

To try a catalog compiler instead of the canonical one, pass `--compiler` with
a catalog ID, e.g. `bin/harness lift flag-search TARGET@0xADDRESS --compiler gcc-2.8.0-psx`.
Its output is diagnostic only: a non-exact result never retains an object
override, and even a fresh exact result needs a reviewed
`BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` entry in `config/compiler/object-flags.cmake`
before the build selects it.

### 6. Promote duplicate knowledge

Follow the evidence gate and ownership model in
[function matching: Exact duplicate groups](matching.md#exact-duplicate-groups).
It is the normative duplicate-promotion procedure; every wrapper remains
address-owned and independently validated.

### 7. Audit and hand off

```sh
bin/harness build TARGET@0xADDRESS
bin/harness build TARGET
bin/harness lift status TARGET --detail normal
just check
git diff --check
```

`build` compiles authored objects; it does not reconstruct a complete image.
`bin/harness lift status --detail minimal` prints totals, `normal` adds target
totals and invalid details, and `full` prints every function. `just check` runs
lint, documentation drift, text-tool tests and corpus checks, symbol validation
and source validation. `just check-all` adds the full Python suite. Neither
command supplies live acceptance evidence for an individual lift.

On a cold or partially invalidated cache, `bin/harness lift status` batch-builds all
valid cache-miss objects per owning target in a single CMake invocation, then
compares each individually. An all-cache-hit target issues no build command.
Configuration belongs to the shared batch builder; status does not repeat it.
Disposable status cache v7 keys entries by canonical target, source path and function
address. Cached payload identity must match that key; malformed or misbound records
are misses, and source-only schemas invalidate rather than promote old rows. Target
and whole-source fingerprints retain their existing input coverage. This does not
authenticate arbitrary cached result values or permit grouped native comparison;
the preflight guard still rejects grouped sources before cache reuse.
Post-build ownership refresh and per-source fallback remain mandatory.
After every successful or failed batch, status reloads manifests. Missing, changed
or unreadable target metadata yields uncached invalid records, without comparison
or fallback using the queued model. Unchanged models retain fallback. This
boundary check does not prove continuity through later
native steps.
The cache also binds the supplied target model, including
nested placements and companion metadata loaded from alternate TOML names, plus
object flags and the variant catalog.
Those two files use bounded confined reads (4 MiB each); missing and empty differ,
and unsafe paths or failed reads reject. Dictionary order is ignored; sequence order
is preserved. This is not atomic input capture or complete compiler/runtime closure.
`--no-cache` bypasses disposable audit summaries but still batch-builds selected
valid misses; it is for diagnosis, not lift acceptance. Cache rows are never
used for acceptance: immediately before accepting a lift, run live
`asm-diff`, `byte-match`, `companion-check` where relevant, `splat`, and
`symbols check`.

For iteration, select the affected target instead of repeatedly auditing every
target; retain repository-wide handoff checks. `--detail` only changes displayed
output, not validation work. Profile warm-cache and `--no-cache` runs separately:

```sh
PYTHONPATH=tools/python .venv/bin/python -m cProfile -o /tmp/bof3-status.prof -m harness.commands.decomp_status TARGET --no-cache --detail minimal
.venv/bin/python -c 'import pstats; pstats.Stats("/tmp/bof3-status.prof").strip_dirs().sort_stats("cumulative").print_stats(25)'
```

`--no-cache` bypasses status summaries, not native build caches. Manifest reuse
stays within the current operation; comparison reloads ownership after building.
Each manifest-validation pass rechecks every claim's contents and canonical path.
Within that pass, shared parent paths are resolved once and rechecked at the end;
leaf symlinks retain strict resolution. This reduces repeated ancestor traversal,
not content hashing or post-build freshness checks. Measure manifest setup
separately from native build, link and comparison time; setup speedups alone do
not establish an end-to-end status speedup.

## Command ownership

| Command | Why it exists | Primary artifacts |
| --- | --- | --- |
| `bin/harness media disc` | inspect/extract original disc files | chosen output |
| `bin/harness emi archive` | list, extract, or explicitly repack EMI archives | chosen output |
| `bin/harness media str` | inspect, validate, or convert STR media | chosen output |
| `bin/harness emi target` | preview/create one EMI target | only with `--apply` |
| `bin/harness lift companion-check` | gate a lift through a declared EMI companion call | JSON readiness report |
| `bin/harness build` | compile all, one target, or one function | `build/` |
| `bin/harness source splat` | regenerate reviewed segment output | `out/splat/` |
| `bin/harness analysis spimdisasm` | disassemble MIPS images directly | terminal output or explicit destination |
| `bin/harness source symbols` | map check/normalize, bindings, PsyQ import/bindings/report | explicit subcommand |
| `bin/harness analysis rizin` | pinned local Rizin analyzer | terminal output only |
| `bin/harness analysis rz-project` | isolated Rizin analyze/status/open | `out/reverse/snapshots/` on analyze |
| `bin/harness analysis index` | rebuild the fresh cross-target query cache | `out/index/` |
| `bin/harness analysis query`, `bin/harness analysis readiness` | query fresh indexed evidence and bounded aggregate readiness | none |
| `bin/harness naming`, `types`, `macros`, `combiner`, `source metadata` | list current facts and check mechanical consistency and native consumers | console or JSON results and disposable native outputs |
| `bin/harness lift m2ctx`, `bin/harness lift m2c` | generate target context and C seed | `out/` or `-o` |
| `bin/harness lift asm-diff`, `bin/harness lift byte-match` | compare one authored lift | `out/asm-diff/`, `out/matching/`, `out/bindings/`, `build/` |
| `bin/harness lift flag-search` | rank known compiler flag profiles | report plus `out/matching/` baseline |
| `bin/harness lift permute` | bounded source-shape search | `out/permuter/` |
| `bin/harness lift promote` | validate canonical candidate | generated comparison only |
| `bin/harness lift status` | audit exact/partial/invalid lifts | `out/matching/`; full JSON with `-o` |
| `bin/harness psyq import` | stage PsyQ build headers | explicit destination |
| `bin/harness psyq` | scan/calls/proposal PsyQ signature evidence | `out/psyq/` |
| `bin/harness build variants` | resolve/stage per-object compiler variants | `toolchains/gcc-variants/` |
| `bin/harness source validate` | validate source ownership and generated snapshots | report only |
| `bin/harness lift gate` | combined per-selector native gates | terminal PASS/FAIL |
| `bin/harness lift sweep` | measure clean-C candidate shapes in one call | terminal table |
| `bin/harness lift next` | rank the next lift candidate | terminal or `--json` |
| `bin/harness lift campaign` | lift campaign bookkeeping | `out/reviews/lift-campaign/` |
| `bin/harness agent context` | assemble bounded agent mission context | terminal |
| `bin/harness decomp diagnose` / `audit` | parent lift diagnosis and retained candidate audit | retained evidence |
| `bin/harness audio build` / `package` | Cargo release build / Rust source package | `out/audio/release/bof3-audio` / `out/bof3-audio-source.zip` |
| `bin/harness audio index` / `query` / `map` / `extract` / `render` / `pack` / `verify` | invoke Rust audio operations; native `--help` describes supported inputs | selected output location |
| `just setup --component bios` / `--component pcsx-redux` | prepare pinned US BIOS / build PCSX-Redux Agentic with local SDL3 | `inputs/external/bios/scph5501.bin` / `out/setup/pcsx-redux.json` |
| `bin/harness setup --component psyqo` | build the native Nugget toolchain and PSYQo SDK | `toolchains/psyqo/`, `build/third_party/psyqo-toolchain/` |
| `bin/harness setup --component rz-psxdec` | build the sibling native decompiler forks | `build/third_party/rz-psxdec/` |
| `bin/harness reconstruction` | prepare, generate and compare bounded original-only proposals | fresh selected run directory; [reconstruction guide](../../tools/retdec-psx/reconstruction.md) |
| `bin/harness runtime` | discover embedded emulator missions and execute bounded captures | fresh selected output directory |

`just doctor` scans `inputs/external/bios/` for a regular file matching the US
BIOS SHA-256 embedded in the BIOS tool, regardless of name or extension. It needs
no inventory or collection config and neither downloads nor repairs files.

Audio `build` without arguments builds and prints the executable path; optional
operation arguments run it. Packaging includes both local Rust crates, lockfiles,
tests and dependency licenses, excluding proprietary media. TUI, live playback,
standalone PSF commands and FLAC are retired from this command surface. Full PSX
rendering and MP3/Ogg delivery remain incomplete; host C source is retained until
migration acceptance. [Reference setup](../reference/audio-runtime-setup.md)
documents BIOS management and SDL3 policy. [Emulator integration](pcsx-redux-agentic.md)
covers the fork build, embedded missions and skill installation.

The shared panel-task implementation templates live at `include/ui/panel_task_templates.h`; target-local wrappers compile them and retain symbol/address ownership.

Every command is one `bin/harness <domain> <action> [subaction]` invocation.
`bin/harness --help` lists the domains and actions, `bin/harness <domain>
--help` lists a domain's actions, `bin/harness <domain> <action> --example`
prints a minimal invocation, and `bin/harness <domain> <action> --help`
prints the owning parser's options. `--root`, `--example` and error/exit
handling come from the shared `harness.common.cli` owner; `bin/harness` is
dispatch only and every domain keeps its own policy and parser.

`bin/cc`, `bin/as`, `bin/ld`, `bin/ar`, `bin/nm`, `bin/objcopy`, `bin/objdump`,
`bin/ranlib`, `bin/strip`, and `bin/maspsx` remain build/toolchain adapters:
CMake and the compiler pipeline invoke `bin/cc`/`bin/as` by absolute path,
`just setup` verifies the rest, and `bin/python-env` is the one sourced shared
bootstrap. Workflow users call the `bin/harness` commands, not the adapters.

`bin/cc` uses the historical GCC 2.7.2-PSX and maspsx pipeline. The binutils
adapters use `toolchains/psyqo/bin/mipsel-none-elf-*`. Explicit compiler variants
remain available through `bin/harness build variants`.

See [function matching](matching.md) for C iteration rules,
[build analysis evidence](#3-build-analysis-evidence) for analyzer contracts, and
[project context](project-context.md) for ownership.

## OpenSpec

```sh
openspec list
openspec status --change CHANGE_NAME
```

Use `$openspec-propose` to create planning artifacts, `$openspec-continue-change`
to add the next artifact, and `$openspec-update-change` to revise an existing
change. Use `$openspec-apply-change` for implementation after planning review.
Keep task status in its owning change. Artifact status does not establish source
acceptance; BOF3 evidence, native gates, independent review and rollback still apply.
