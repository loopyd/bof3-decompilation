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
bin/rev-query hotspots
bin/rev-query status
```

`just index` atomically replaces `out/index/reverse.sqlite` only when every
snapshot is fresh and complete. Use `bin/rev-query --help` for query variants
and `--json` for structured output.

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
