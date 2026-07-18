# Rizin and reverse index

Rizin output is target-qualified generated evidence. Original bytes, manifests,
reviewed Splat layouts, maps, and annotations remain authoritative.

## Bootstrap one EMI entry

```sh
bin/emi-target BIN/BATTLE/BATL_END.EMI#0
bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply
```

Preview is read-only and deterministic. `--apply` revalidates identity and
creates a normalized image and metadata, target manifest, bin-only Splat
layout, and empty target-local map. It refuses existing targets and never
infers code or creates C.

## Analyze one target

```sh
bin/rz-project analyze TARGET
bin/rz-project status TARGET
bin/rz-project open TARGET
```

The command composes the manifest, Splat roots, target-local map, and reviewed
overlay in memory. Analysis fails closed on command or JSON errors. Never mix
overlapping target mappings in one session.

## Build and query the index

```sh
just index
bin/rev-query symbols NAME
bin/rev-query xrefs TARGET@0xADDRESS
bin/rev-query calls TARGET@0xADDRESS
bin/rev-query duplicates
bin/rev-query metrics TARGET@0xADDRESS
bin/rev-query quick-wins --unlifted --detail minimal --limit 5
bin/rev-query hotspots --detail minimal --limit 5
bin/rev-query leafs --unlifted --detail minimal --limit 5
bin/rev-query pareto --unlifted --detail normal
bin/rev-query status
```

`just index` atomically replaces `out/index/reverse.sqlite` only when every
snapshot is fresh and complete. Use `bin/rev-query --help` for query variants
and `--json` for structured output.

Ranking queries expose their raw inputs: instructions, CFG blocks/edges,
cyclomatic complexity, loops, stack/locals/arguments, callers/callees,
unresolved calls, and exact-byte duplicate leverage.
`quick-wins` orders low-effort candidates; `hotspots` orders impact;
`pareto` returns candidates not dominated on the visible effort/value axes.
`leafs` collapses recursion and reports `analyzer_no_edge`, `unresolved_edge`,
or `non_leaf`; it never treats analyzer silence as proof. Derived fields carry a
`score_version`, while JSON remains the stable automation output.
Exact `jr ra; nop` return stubs remain visible in `metrics` and `duplicates`
but are excluded from rankings unless `--include-trivial` is explicit.

`duplicates` groups exact analyzer-range bytes by hash and size, chooses a
deterministic representative, and reports unlifted members and estimated saved
instructions. Each member still requires target-local byte validation; shared
bytes do not transfer names, types, or ownership.

## PsyQ evidence

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

The generated `out/psyq/index.json` records target-local complete-object
signature matches and matching SDK versions. `out/psyq/calls.json` joins those
matches to snapshot callsites. Treat both as disposable evidence.

| Source | Evidence |
| --- | --- |
| Pinned PsyQ signatures | Object identity, labels, addresses, matching versions |
| Official PsyQ 4.7 headers | C declarations, types, constants, macros |
| Rizin snapshots | Callsites and xrefs |

Preserve all matching versions. A signature match does not prove one SDK
version, authorize cross-target address reuse, or license a PsyQ source lift.

## Rules

- Keep separate targets when bytes, addresses, or names coincide.
- Keep raw names until semantics are reviewed.
- Store reviewed findings in `docs/specs/`; keep raw exports and hypotheses
  under `out/`.
