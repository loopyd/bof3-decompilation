# Prepare the reconstruction corpus

`bin/harness reconstruction prepare` verifies the fixed corpus and current fork
build, then writes separate generator and evaluator inputs under a fresh run.
It reconstructs no function and grants no exactness credit.

## Current inputs

The maintained [corpus manifest](corpus.json) fixes 118 admitted target-qualified
identities, original images and complete byte ranges, plus seven separate exclusions.
Its evaluator reference records contain authored names, signatures, source locations
and source/map hashes. Preparation rejects a changed manifest, image, range,
reference or current native build input. It validates alignment, image format,
PS-X headers and payload bounds without an old collector or audit directory.

Build the sibling forks through the [managed lifecycle](README.md). Use the receipt
path printed by setup. Run from the repository root with a new run id matching
`[a-z0-9][a-z0-9._-]{0,63}`:

```sh
bin/harness reconstruction prepare \
  --run original-inputs \
  --native-receipt "$native_receipt"
```

`--baseline` defaults to `tools/retdec-psx/corpus.json`. Global `--root` appears
before `prepare` and defaults to `out/retdec-118-reconstruction`.
`--json` prints the command result. Successful preparation returns exit 0;
refused inputs return exit 2. Unexpected I/O failures can return another nonzero exit.

## Separate generator inputs from answers

The new run contains these bundles:

| Bundle | Contents |
| --- | --- |
| `problem/` | Admitted identities, complete original images, reviewed ranges, loading metadata and generic rules. |
| `answer/` | Evaluator-only reference metadata, pinned authored sources/maps, exclusions, corpus and current fork identities. |
| `preparation/` | The verified Problem and Answer bindings. |

The Problem is built through a field allowlist. Authored names, signatures, source
locations, layouts, maps, compiler selections and results cannot enter it.
The empty `rules.json` supplies no authored PSX rules. Native recovery lives in the
customized plugin. Reference profiles and historical results remain absent.

The generator receives original images and explicit request/configuration inputs.
It does not receive the Answer bundle. Associated data, placement, relocations and
external bindings start unresolved and require independent evaluator evidence.
Omitting them from a candidate cannot remove an acceptance obligation.

## Publication and privacy

Preparation creates a new directory exclusively, verifies written bytes and makes
published input files read-only. An existing directory is refused. A crash or
publication failure can leave partial files; a run without the final preparation
binding is unusable. Retry in a fresh directory.

Original game bytes and captures stay local in ignored `out/`. Do not publish
prepared bundles or private media. Obsolete runs, audit snapshots and disposable
checks can be removed after retaining current inputs and requested deliverables.
See [reconstruction runs](reconstruction.md) for generation, verification and reports.
