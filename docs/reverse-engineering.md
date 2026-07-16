# Rizin and reverse index

Rizin evidence is target-qualified, reproducible, and disposable. Tracked
layouts, maps, replay, and original bytes remain authoritative.

## Per-target Rizin project

```sh
bin/rz-project rebuild TARGET
bin/rz-project analyze TARGET
bin/rz-project status TARGET
bin/rz-project export TARGET
```

One generated project exists under `out/rizin/<target>/` for each binary. Never
open multiple overlapping load mappings in one project. `rebuild` imports the
target manifest, Splat boundaries, canonical map, and reviewed replay.

`analyze` is bounded by default. `analyze --deep` creates candidate evidence
only. Use the generated snapshot and deterministic export to review facts;
native database state is not durable evidence.

`export TARGET` prints the deterministic patch. `export TARGET --write` is the
only path that changes reviewed replay after validation. `open TARGET` starts
an isolated interactive Rizin session.

## Index and queries

```sh
just index
bin/rev-query symbols NAME
bin/rev-query xrefs TARGET@0xADDRESS
bin/rev-query calls TARGET@0xADDRESS
bin/rev-query duplicates
bin/rev-query hotspots
bin/rev-query leafs
bin/rev-query variables
bin/rev-query status
```

`just index` rebuilds `out/index/reverse.sqlite` atomically from fresh,
complete target exports. It fails when evidence is missing, stale, or
incomplete, leaving the last complete index intact.

The cache contains target-local symbols, functions, calls, xrefs, data
references, exact hashes, duplicate groups, PsyQ evidence, and project
metadata. `bin/rev-query` is its only query surface; pass `--json` for stable
structured output.

## Psy-Q signatures

The pinned `toolchains/psx_psyq_signatures/` submodule is an object-signature
database, not a source of C declarations. Initialize it, then scan every
current target manifest and join the matches to target-local analyzer calls:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

`bin/harness psyq` is a deliberately limited compatibility adapter; the
focused tools remain the normal command surface. `scan --all` reads each
manifest's image and load address, including the executable targets and every
promoted EMI binary target. It checks complete four-byte-aligned `LIB` and
`OBJ` signatures for Psy-Q versions `3610`, `3611`, `370`, `400`, `410`, `420`,
`430`, `440`, `450`, `460`, and `470`, then merges identical target/address/
object results across versions.

The generated `out/psyq/index.json` records target-local address, library,
object, matching versions, and recovered label symbols. `calls --all` reads
the existing Rizin snapshot xref evidence and writes matching callsites to
`out/psyq/calls.json`. Both files are disposable generated evidence: never
edit or commit them.

Keep these sources separate:

| Source | Owns |
| --- | --- |
| Psy-Q signatures | Matched objects, symbols, and target-local addresses. |
| Psy-Q 4.7 headers | C types, prototypes, constants, and macros. |
| Rizin snapshots | Callsites and xrefs joined by `calls --all`. |

Preserve every matching SDK version. A match does not prove that BOF3 used one
of the available signature versions, and it never authorizes a cross-target
address reuse or a Psy-Q source lift.

## Evidence rules

- Retain separate targets even when their bytes, addresses, or names coincide.
- Keep raw `func_XXXXXXXX` and `D_XXXXXXXX` names until semantics are reviewed.
- Rizin and decompiler output support a claim; they never override bytes,
  target mapping, reviewed Splat layout, or C matching.
- Put reusable, reviewed conclusions in `docs/specs/`; keep raw exports and
  transient hypotheses under `out/`.
