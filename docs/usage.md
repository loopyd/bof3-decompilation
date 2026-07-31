# Tool usage

Use one target-qualified function at a time. Commands default to bounded human
output; generated evidence under `out/` retains full detail.

## Output budget

Context-heavy commands accept `--detail minimal|normal|full`:

| Level | Use | Output |
| --- | --- | --- |
| `minimal` | candidate scouting, small models | decision fields or one summary line |
| `normal` | daily iteration | labeled metrics or the first bounded diff hunk |
| `full` | debugging/tool development | complete rows, function records, or diff |

`normal` is the text default. Existing `--json` without `--detail` remains full
for automation compatibility. `-o FILE` always writes the complete artifact.
Do not truncate payload commands such as `m2c`; write them with `-o` and load
only the relevant file when editing.

## Ordered workflow

### 1. Prepare the repository

```sh
just setup
just doctor
```

`setup` validates one complete CUE/BIN set under `inputs/external/`. If it is
not present, it accepts `inputs/external/BreathOfFireIIIv1.1.7z` and extracts it
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

```sh
# Gather provenance (signatures identify objects; official headers own declarations)
bin/psyq-import --example
bin/harness psyq scan --all
bin/harness psyq calls --all
bin/harness psyq proposal --all

# Apply reviewed provenance, then regenerate and audit bindings
bin/symbols import-psyq out/psyq/proposal.json --all-qualified --write
bin/symbols psyq-bindings --write          # regenerate src/<t>/symbols/psyq.c
bin/symbols psyq-report TARGET             # which SDK symbols the code references
```

The SDK maps live in `config/sdk/psyq-{slus,logo}.txt`; a target selects its
space via the manifest `[psyq] space` key (default `slus`). `import-psyq` writes
reviewed exact candidates into the space's SDK map; `psyq-bindings` regenerates
the compiled `src/<t>/symbols/psyq.c` from it. Nothing edits maps without
`--write`.

### 3b. Analysis sequence (freshness → rebuild → query)

```sh
bin/analysis-sequence TARGET --ranking quick-wins
bin/analysis-sequence TARGET --ranking metrics TARGET@0xADDRESS --detail normal
```

`analysis-sequence` checks `bin/rz-project status TARGET` first; fails with the
target and `snapshot` stage when stale; rebuilds the index only after freshness
succeeds; runs the requested `rev-query` ranking without touching other targets
or reviewed maps. `loop-status.py` is inspection-only by default; use
`--recover` to repair stale generated snapshots/indexes before it ranks
candidates. `rev-query` refuses stale snapshot/index evidence. Use `--exclusions`
to inspect rows rejected by canonical-code checks; use `--detail full` for
complete rows.

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
bin/rev-query symbols NAME
bin/rev-query variables NAME
bin/rev-query status
```

`bin/rev-query mission TARGET@0xADDRESS` composes a single-function lifting
brief (metrics, callers/callees, duplicate group, SDK callees, and risk flags) —
the input to the autonomous lift loop. To lift a batch unattended, run
`/skill:bof3-lift-loop` (see `.agents/skills/bof3-lift-loop/README.md`); it
gates each exact match through a reviewer and commits only reviewed lifts.

Addresses are target-qualified where identity matters; overlapping addresses
in different images never share query results.

### 5. Lift and iterate

```sh
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit src/<target>/func_XXXXXXXX.c and adjacent target evidence
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
bin/permute TARGET@0xADDRESS --time-limit 300 --quiet -j N
bin/promote TARGET@0xADDRESS src/<target>/func_XXXXXXXX.c --detail normal
```

`flag-search` suggests compiler flags from known profiles. `permute` searches
source shapes in a disposable workspace. `promote` validates the canonical
source but never edits source, maps, or layouts.

To try a catalog compiler instead of the canonical one, pass `--compiler` with
a catalog ID, e.g. `bin/flag-search TARGET@0xADDRESS --compiler gcc-2.8.0-psx`.
Its output is diagnostic only: a non-exact result never retains an object
override, and even a fresh exact result needs a reviewed
`BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` entry in `config/compiler/object-flags.cmake`
before the build selects it.

### 6. Promote duplicate knowledge

1. Verify group bytes and reviewed boundaries.
2. Make one representative byte-match.
3. Independently make a second member byte-match.
4. Normalize evidence-backed roles, types, fields, and constants.
5. Share a cross-target embedded body under `src/shared/<domain>/` only when
   repeated maintenance justifies an `.inc`;
   retain every address-owned wrapper and target-local check.

An implementation embedded in several EMIs is compile-time source reuse, not
an engine service. A real engine service exists once in `SLUS_004.22`; keep its
implementation in `src/exe/slus_004_22/` and put only its proven public contract
in `include/bof3/core.h`. `src/shared/` templates compile into each owning
image; do not create an orphan `src/engine/` or link one EMI against another.

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
| `bin/rz-project` | isolated Rizin analyze/status/open | `out/reverse/` on analyze |
| `bin/index` | rebuild the fresh cross-target query cache | `out/index/` |
| `bin/rev-query` | query fresh indexed evidence | none |
| `bin/m2ctx`, `bin/m2c` | generate target context and C seed | `out/` or `-o` |
| `bin/asm-diff`, `bin/byte-match` | compare one authored lift | `out/asm-diff/`, `out/matching/`, `out/bindings/`, `build/` |
| `bin/flag-search` | rank known compiler flag profiles | report plus `out/matching/` baseline |
| `bin/permute` | bounded source-shape search | `out/permuter/` |
| `bin/promote` | validate canonical candidate | generated comparison only |
| `bin/decomp-status` | audit exact/partial/invalid lifts | `out/matching/`; full JSON with `-o` |
| `bin/psyq-import` | stage PsyQ build headers | explicit destination |
| `bin/harness psyq` | scan/calls/proposal PsyQ object signatures | `out/psyq/` |

`bin/cc`, `as`, `ld`, `ar`, `nm`, `objcopy`, `objdump`, `ranlib`, `strip`, and
`maspsx` are build adapters. Workflow users should call `bin/build` and the
matching commands instead of invoking these adapters directly.

See [matching](matching.md) for C iteration rules, [tool usage](usage.md)
for analyzer contracts, and [context](../CONTEXT.md) for ownership.
