# Rizin and reverse index

Rizin evidence is target-qualified, reproducible, and disposable. Tracked
layouts, maps, replay, and original bytes remain authoritative.

## Bootstrap one extracted EMI entry

```sh
bin/emi-target BIN/BATTLE/BATL_END.EMI#0
bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply
```

The default prints the exact bin-only target plan. `--apply` revalidates the
payload identity and creates only the normalized image and identity metadata,
manifest, Splat layout, and empty target-local map. It refuses existing targets
and never infers code ranges or creates C source.

## Per-target Rizin analysis

```sh
bin/rz-project analyze TARGET
bin/rz-project status TARGET
bin/rz-project open TARGET
```

The command composes the target manifest, Splat boundaries, canonical map, and
reviewed overlay in memory. Never open multiple overlapping load mappings in
one session.

`analyze` uses accepted roots before one bounded analysis pass and writes a
snapshot only when every required JSON query succeeds. Native database state
is not durable evidence; `open TARGET` starts an isolated interactive session.

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
references, exact hashes, duplicate groups, and PsyQ evidence
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
`OBJ` signatures for every pinned database release: `260`, `300`, `330`,
`340`, `350`, `3610`, `3611`, `370`, `400`, `410`, `420`, `430`, `440`, `450`,
`460`, and `470`, then merges identical target/address/object results across
versions.

The scanner also writes an exact-name catalog of the official Psy-Q 4.7 header
baseline under `out/psyq/4.7/headers.json`. This catalog can attach declaration
evidence to a signature label, but headers do not prove an object identity and
signature labels do not license a declaration or map edit by themselves.

The generated `out/psyq/index.json` records target-local address, library,
object, matching versions, recovered label symbols, and per-target
best-compatible-version counts. It separately ranks the repository's primary
historical comparison window (`3610`, `3611`, `370`, `400`) and records `410`
as a possible regional-rebuild comparison. These are review priorities, not
provenance claims. It also records complete-object matches that do not support
the most-compatible version as discrepancies for review. `calls --all` reads
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

The index also exposes an alignment score for every scanned release. A complete
object matching `N` releases contributes `1/N` to each, so version-specific
objects carry more weight than unchanged library code. Use that score together
with the historical window; neither score establishes an SDK provenance claim.

## Evidence rules

- Retain separate targets even when their bytes, addresses, or names coincide.
- Keep raw `func_XXXXXXXX` and `D_XXXXXXXX` names until semantics are reviewed.
- Rizin and decompiler output support a claim; they never override bytes,
  target mapping, reviewed Splat layout, or C matching.
- Put reusable, reviewed conclusions in `docs/specs/`; keep raw exports and
  transient hypotheses under `out/`.
