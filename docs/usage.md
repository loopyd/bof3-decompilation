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

## Ordered workflow

### 1. Prepare the repository

```sh
just setup
just doctor
```

First-time setup requires the host tools listed in the
[README prerequisites](../README.md#prerequisites); see that list for which
`cmake`/`7z` roles are required and why. `setup` validates one complete CUE/BIN
set under `inputs/external/`. If it is not present, it accepts `inputs/external/BreathOfFireIIIv1.1.7z` and extracts it
to the private-assets cache. It then downloads/stages the required toolchains,
extracts reviewed target images, and validates the result. `doctor` repeats
setup validation. Run `bin/symbols check` separately to validate symbol maps.

Use `bin/bof3-disk` to inspect original disc media, `bin/emi-ex` to list or
extract an EMI archive, and `bin/str-media inspect|validate|convert` for STR
media. These are acquisition tools, not function-analysis inputs.

### 2. Bootstrap one new EMI target

```sh
bin/emi-target BIN/BATTLE/BATL_END.EMI#0
bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply
bin/symbols check
bin/splat TARGET
```

`emi-target` previews before `--apply`; it refuses an existing target and
creates a new identity plus bin-only reviewed layout. `symbols` validates
target-local maps. `splat` regenerates assembly and linker inputs for new or
existing reviewed targets; add `--verbose` only for complete Splat diagnostics.

### 3. Build analysis evidence

```sh
bin/rz-project analyze TARGET
bin/rz-project status TARGET
just index
```

`rz-project` keeps each independently loaded image isolated. Analyze every
stale or missing target snapshot before `just index`; indexing fails unless all
manifest snapshots are fresh, then atomically rebuilds the cross-target cache.
Use `bin/rz-project open TARGET` only for interactive investigation.

PsyQ SDK evidence and application:

`bin/harness` is the permanent, narrow PsyQ object-signature evidence adapter.
Its only command family is `psyq {scan|calls|proposal}`; symbol-map mutation
remains under `bin/symbols`.

```sh
# Gather provenance (signatures identify objects; official headers own declarations)
bin/psyq-import --example
bin/harness psyq scan --all
bin/harness psyq calls --all
bin/harness psyq proposal --all

# Apply reviewed provenance, then regenerate and audit bindings
bin/symbols import-psyq out/psyq/proposal.json --all-qualified --write
bin/symbols psyq-bindings --write          # regenerate manifest-owned psyq_source bindings
bin/symbols psyq-report TARGET             # which SDK symbols the code references
```

The SDK maps live in `config/sdk/psyq-{slus,logo}.txt`; a target selects its
space via the manifest `[psyq] space` key (default `slus`). `import-psyq` writes
reviewed exact candidates into the space's SDK map; `psyq-bindings` regenerates
the compiled manifest-owned `psyq_source` (e.g. `src/bof3/support/slus_psyq.c`)
from it. Nothing edits maps without `--write`.

### 3b. Target analysis (freshness → rebuild → query)

Run the existing commands explicitly, in order:

```sh
bin/rz-project status TARGET
bin/index
bin/rev-query quick-wins --target TARGET
# Or: bin/rev-query metrics TARGET@0xADDRESS --detail normal
```

Stop if `rz-project status` reports a stale snapshot; rebuild that target's
snapshot before running `bin/index`. Rebuild the index only after freshness
succeeds, then query the requested target without touching reviewed maps. The lift-loop status script
(`python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py`) is inspection-only
by default; use `--recover` to repair stale generated snapshots/indexes before
it ranks candidates. `rev-query` refuses stale snapshot/index evidence. Use `--exclusions`
to inspect rows rejected by canonical-code checks; use `--detail full` for
complete rows.

### Agent context prefill

Every project agent starts repository work with exactly one bounded, read-only
prefill:

```sh
bin/agent-context ROLE [TARGET@0xADDRESS]
bin/agent-context cleanup audit-target TARGET
bin/agent-context cleanup retained-lift TARGET TARGET@0xADDRESS exact [ROW...]
```

The default `stable` mode emits tracked role rules and, when selected, tracked
target-owned facts; it never includes generated `out/` evidence. Cleanup accepts
only the seven canonical forms documented by `bof3-cleanup`; its structured
request selects and loads exactly one cleanup skill body plus route-owned direct
references. The temporary parent-only `audit docs/...` form requires
`--parent-compatibility`; other old audit inputs are rejected. This bounded
compatibility remains intentionally active after implementation closeout; it is
not incomplete work. Remove it only after one tagged project release or 30
consecutive qualifying parent cleanup sessions, with zero tracked callers and
independent review. Do not rerun a prefill or reread emitted paths without a
named evidence gap. `--mode compatibility` exists only for full legacy-output
diagnostics and may include optional generated evidence; agents must not use it
as their prefill.

### 4. Select one function

```sh
bin/rev-query quick-wins --unlifted --detail minimal --limit 5
bin/rev-query leafs --unlifted --detail minimal --limit 5
bin/rev-query duplicates --unlifted --detail normal --limit 5
bin/rev-query metrics TARGET@0xADDRESS --detail normal
bin/rev-query quick-wins --exclusions --detail full --limit 0
```

Use `--exclusions` on any ranking command to inspect target-qualified rows
rejected by canonical code checks; it reports `candidate_exclusion` instead of
ranked candidates. Use `quick-wins` for low effort, `hotspots` for caller impact, `leafs` for
bounded call dependencies, `pareto` for visible effort/value trade-offs, and
`duplicates` for exact-byte leverage. Rankings are hints, not promotion proof.

Supporting queries:

```sh
bin/rev-query calls TARGET@0xADDRESS
bin/rev-query xrefs TARGET@0xADDRESS
bin/rev-query owners TARGET@0xADDRESS
bin/rev-query describe TARGET@0xADDRESS
bin/rev-query transaction-scope TARGET SYMBOL
bin/rev-query inventory TARGET
bin/rev-query symbols NAME
bin/rev-query variables NAME
bin/rev-query types [NAME] [--target TARGET] [--untyped] [--detail full]
bin/rev-query type-uses [NAME] [--target TARGET]
bin/rev-query type-candidates [--target TARGET] [--status blocked] [--kind KIND]
bin/rev-query macros [NAME] [--target TARGET] [--classification KIND]
bin/rev-query macro-uses [NAME] [--target TARGET]
bin/rev-query macro-opportunities [--target TARGET] [--kind KIND]
bin/rev-query near-duplicates [--target TARGET]
bin/rev-query status
```

`owners` combines reviewed Splat ranges, analyzer ranges, and exact mapped-entry leads; provenance and confidence are hypotheses, never ownership authority. `xrefs` includes call references and decoded data accesses with source address, access kind, and opcode. `describe` reports canonical payload/file offsets, reviewed Splat boundary, exact symbol, and references. `types` inventories target-owned declarations plus the explicitly shared base scalar aliases; full detail includes fields, layout constraints, conflicts, diagnostics, and provenance. `type-uses` reports declaration/use relationships. `type-candidates` reports conservative representation and semantic leads only: every inferred aggregate, field, array, prototype, or class-like receiver/dispatch row remains blocked until its independent evidence gaps are closed. `macros` inventories definitions from manifest-claimed headers/sources plus sanctioned shared helpers and `src/shared/**/*.inc`; `macro-uses` reports target-qualified lexical expansions and restrictions. Generated PsyQ binding uses are `noncandidate`/`generator_owned`.

`macro-opportunities` ranks read-only, target-qualified leads for repeated constants, expression/accessors, token-exact three-statement windows, and reviewed exact groups. `near-duplicates` reports reviewed non-trivial functions only when instruction count, CFG metrics, call/data-reference shape, and normalized instruction shape agree and all deltas are immediate/address operands (branch displacements remain exact). Every row stays `blocked`, includes evidence, counterexamples, source-level semantic guards, and fingerprints or reviewed hashes; cross-target clusters are report-only. Generated inputs, stale source/binary evidence, embedded data, trivial stubs, and analyzer/reviewed boundary disagreement fail closed or are excluded. These queries never edit source or promote a candidate.

`bin/analysis-readiness [TARGET]` is the bounded aggregate checkpoint. By default it reports snapshot/index freshness and stale facts, then summary work graphs and exact naming, type, and macro counts; `TARGET` restricts every inventory, debt, candidate, and work count to that target. Use `--detail full` only when the exhaustive candidate rows, blockers, fingerprints, and generated naming work are required. Both modes retain the `bof3.analysis-readiness/v2` schema and differ only in `work_graph` detail. The command is read-only and prints `bin/index --recover` when authoritative inputs have made the disposable index stale. Recovery is explicit so reviewed transactions pass before index refresh.

Naming audits start with readiness preflight, then use `bof3.naming-audit/v3` typed rungs, generated required work, typed corroborators, canonical transaction scope/storage, and digest-verified receipts. A mechanically safe exact-progress repair requires live proof:

```sh
bin/naming-audit prepare TARGET
bin/naming-audit prepare TARGET --repair
bin/naming-audit init TARGET out/reviews/audit.json
bin/naming-audit validate TARGET out/reviews/audit.json --transaction function:func_80100000
# Apply the isolated spelling transaction, then verify current truth:
bin/naming-audit verify TARGET out/reviews/audit.json --transaction function:func_80100000
bin/naming-audit validate TARGET out/reviews/audit.json
```

`init` writes every current raw inventory row once as an explicit blocked evidence gap, including tool-generated required work and the next bounded command for each open typed rung; auditors replace those rows only with receipt-backed exhausted or proposed conclusions. The isolated pre-transaction check ignores unrelated blocked rows but rejects incomplete locations, open mandatory work, malformed metadata, noncanonical storage, or overlapping proposals. `bin/naming-audit verify` derives scope by the recorded address and new spelling, proves every reported location migrated, the old spelling is absent, and data storage is unchanged.

Reviewed type applications are concern-isolated and atomic. The disposable reverse index only supplies leads; `prepare` requires a separately reviewed, live-fingerprinted candidate artifact with resolved representation and semantics plus two independent observations. On a dirty worktree, the request must include the exact adopted baseline digest printed by the preflight error/workflow. `run` restricts writes to manifest-owned paths, executes the recorded checks, writes immutable structured receipts, and rolls all changes back on failure:

```sh
bin/type-audit account out/reviews/type-account.json
bin/type-audit validate-account out/reviews/type-account.json
bin/type-audit baseline  # copy digest into adopted_baseline when adopted=true
bin/type-audit prepare out/reviews/type-request.json out/reviews/type-manifest.json
bin/type-audit run out/reviews/type-manifest.json out/reviews/type-changes.json out/reviews/type-application.json
bin/type-audit verify out/reviews/type-application.json --expected-application-digest DIGEST
```

The changes file is a JSON object mapping each allowed repo-relative file to its complete replacement text. Retain the application digest from the `run` output in a trusted external record; do not derive the expected value from the application file being verified. Shared promotion additionally requires two independently verified private application proofs with identical representation and semantic contracts; target-address-bearing contracts are rejected.

Macro application uses the same pinned, atomic workflow:

```sh
bin/macro-audit account out/reviews/macro-account.json
bin/macro-audit validate-account out/reviews/macro-account.json
bin/macro-audit prepare out/reviews/macro-request.json out/reviews/macro-manifest.json
bin/macro-audit run out/reviews/macro-manifest.json out/reviews/macro-changes.json out/reviews/macro-application.json
bin/macro-audit verify out/reviews/macro-application.json --expected-application-digest DIGEST
```

The report records every current macro opportunity ID exactly once as `blocked` or `accepted`, with explicit blocked reasons, per-candidate and source/input fingerprints, and live freshness. `safe_application_count` must remain zero: generated account rows never authorize source changes. `prepare` requires an independently reviewed artifact, manifest-owned private paths or sanctioned shared paths, and exact proofs for shared templates; `run` restricts writes, executes pinned checks, writes receipts, and rolls back on failure. Retain its application digest in a trusted external record before `verify`. The local append-only attestation only detects replacement within this workspace and is not a remote signature or trust anchor.

`bin/rev-query mission TARGET@0xADDRESS` composes a single-function lifting
brief (metrics, callers/callees, duplicate group, SDK callees, and risk flags) —
the input to the autonomous lift loop. To lift a batch unattended, run
`/skill:bof3-lift-loop` (see `.pi/skills/bof3-lift-loop/README.md`); it
gates each exact match through a reviewer and commits only reviewed exact
lifts after explicit user commit authorization.

Addresses are target-qualified where identity matters; overlapping addresses
in different images never share query results.

### 5. Lift and iterate

```sh
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit the metadata-resolved lift source and adjacent target evidence
bin/asm-diff TARGET@0xADDRESS --detail normal
bin/byte-match TARGET@0xADDRESS
```

`m2ctx` materializes target-owned declarations. `m2c` creates a complete seed,
not reviewed C. `asm-diff` prints a bounded diagnostic and keeps the full patch
under `out/asm-diff/`; `byte-match` is the acceptance check. For a function with
an out-of-image companion call, run `bin/companion-check TARGET@0xADDRESS` first;
it exits nonzero until static-call identity, companion boundary/map, reviewed ABI,
and matching caller declaration evidence are all present.

When readable semantics are credible but code shape differs:

```sh
bin/flag-search TARGET@0xADDRESS
bin/permute TARGET@0xADDRESS --time-limit 60 --quiet -j N
```

`bin/data-scan [TARGET...]` lists unlabeled in-image data regions referenced
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
bin/scratchpad preview TARGET@0xADDRESS
bin/scratchpad share TARGET@0xADDRESS
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
a catalog ID, e.g. `bin/flag-search TARGET@0xADDRESS --compiler gcc-2.8.0-psx`.
Its output is diagnostic only: a non-exact result never retains an object
override, and even a fresh exact result needs a reviewed
`BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` entry in `config/compiler/object-flags.cmake`
before the build selects it.

### 6. Promote duplicate knowledge

Follow the evidence gate and ownership model in
[function matching: Exact duplicate groups](agents/matching.md#exact-duplicate-groups).
It is the normative duplicate-promotion procedure; every wrapper remains
address-owned and independently validated.

### 7. Audit and hand off

```sh
bin/build TARGET@0xADDRESS
bin/build TARGET
bin/decomp-status TARGET --detail normal
just check
git diff --check
```

`build` compiles authored objects; it does not reconstruct a complete image.
`decomp-status --detail minimal` prints totals, `normal` adds target totals and
invalid details, and `full` prints every function. `just check` runs repository
tests, lint, maps, and a cached repository audit of retained lifts; it is not
acceptance evidence for an individual lift.

On a cold or partially invalidated cache, `decomp-status` batch-builds all
valid cache-miss objects per owning target in a single CMake invocation, then
compares each individually. An all-cache-hit target issues no build command.
`--no-cache` bypasses disposable audit summaries but still batch-builds selected
valid misses; it is for diagnosis, not lift acceptance. Cache rows are never
used for acceptance: immediately before accepting a lift, run live
`asm-diff`, `byte-match`, `companion-check` where relevant, `splat`, and
`symbols check`.

## Command ownership

| Command | Why it exists | Primary artifacts |
| --- | --- | --- |
| `bin/bof3-disk` | inspect/extract original disc files | chosen output |
| `bin/emi-ex` | list, extract, or explicitly repack EMI archives | chosen output |
| `bin/str-media` | inspect, validate, or convert STR media | chosen output |
| `bin/emi-target` | preview/create one EMI target | only with `--apply` |
| `bin/companion-check` | gate a lift through a declared EMI companion call | JSON readiness report |
| `bin/build` | compile all, one target, or one function | `build/` |
| `bin/splat` | regenerate reviewed segment output | `out/splat/` |
| `bin/spimdisasm` | disassemble MIPS images directly | terminal output or explicit destination |
| `bin/symbols` | map check/normalize, bindings, PsyQ import/bindings/report | explicit subcommand |
| `bin/rizin` | pinned local Rizin analyzer | terminal output only |
| `bin/rz-project` | isolated Rizin analyze/status/open | `out/reverse/snapshots/` on analyze |
| `bin/index` | rebuild the fresh cross-target query cache | `out/index/` |
| `bin/rev-query`, `bin/analysis-readiness` | query fresh indexed evidence and bounded aggregate readiness | none |
| `bin/naming-audit`, `bin/type-audit`, `bin/macro-audit` | validate concern-owned evidence and run explicitly prepared atomic transactions | review artifacts and receipts under the chosen `out/` paths |
| `bin/m2ctx`, `bin/m2c` | generate target context and C seed | `out/` or `-o` |
| `bin/asm-diff`, `bin/byte-match` | compare one authored lift | `out/asm-diff/`, `out/matching/`, `out/bindings/`, `build/` |
| `bin/flag-search` | rank known compiler flag profiles | report plus `out/matching/` baseline |
| `bin/permute` | bounded source-shape search | `out/permuter/` |
| `bin/promote` | validate canonical candidate | generated comparison only |
| `bin/decomp-status` | audit exact/partial/invalid lifts | `out/matching/`; full JSON with `-o` |
| `bin/psyq-import` | stage PsyQ build headers | explicit destination |
| `bin/harness psyq` | permanent narrow scan/calls/proposal PsyQ signature-evidence adapter | `out/psyq/` |

The shared panel-task implementation template lives at `src/shared/ui/panel_task.inc`; target-local wrappers compile it and retain symbol/address ownership.

`bin/cc`, `as`, `ld`, `ar`, `nm`, `objcopy`, `objdump`, `ranlib`, `strip`, and
`maspsx` are build adapters. Workflow users should call `bin/build` and the
matching commands instead of invoking these adapters directly.

See [function matching](agents/matching.md) for C iteration rules,
[build analysis evidence](#3-build-analysis-evidence) for analyzer contracts, and
[project context](agents/project-context.md) for ownership.
