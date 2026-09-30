# Boundary classification evidence

Use `bin/harness analysis boundary` when the reverse index rejects a mapped global
at a reviewed function boundary. It reads manifests, owned headers, source claims,
Splat and original bytes directly; it never opens or repairs the index.

```sh
# Enumerate function/global conflicts across all targets.
bin/harness analysis boundary --json
# Research one mapped address, even when it is not a conflict.
bin/harness analysis boundary emi/etc/commu00/00@0x801F24FC --json
# Compare isolated before/after splits; retain a new JSON report.
bin/harness analysis boundary emi/etc/commu00/00 emi/scenario/scena00/00 emi/world00/area026/13 --probe-textbin -o out/campaign/boundary-review.json
```

A target selector lists its conflicts; `TARGET@0xADDRESS` selects that exact mapped
address. Unknown targets, unmapped selected addresses, missing inputs and input
drift fail. JSON includes declaration provenance, function-candidate reasons,
source claims and lexical mentions, original range/hash/byte previews, proposed
next steps and input hashes. Mentions can include comments; declarations and
previews do not establish the semantics of an entire range. This is not an
exhaustive index preflight, an atomic snapshot or reviewed classification.

## Isolated `textbin` probe

For a finite, unclaimed `asm` range whose only function evidence is its reviewed
boundary and which has a parsed global declaration, `--probe-textbin` tests
`asm → textbin`. The pinned Splat implementation emits a raw `.incbin` in `.text`;
the harness does not classify `textbin` as a function. Unlike moving a range to
`rodata`, this can preserve its section ordering without new harness policy.

The probe copies inputs into a unique `out/boundary-probes/probe-*` workspace,
redirects generated outputs and runs two bounded Splat splits. It retains configs,
logs, linker scripts and extracted bytes. Only supported PSX configuration/segment
shapes are admitted; unknown output options or unsafe names reject. Nothing is
installed, and live source, maps, Splat, snapshots and index are not written.

A `supported` result requires unchanged range offsets/addresses and other
boundaries, byte-identical extracted content, `.text` emission, unchanged `.text`
input order, and identical linker scripts after normalizing the selected object's
path. It **does not** run an assembler/linker, prove whole-image equality, establish
that the range contains no code, or authorize applying the candidate. The retained
`candidate.yaml` is a research artifact, not a drop-in config: relative paths were
originally interpreted from the live configuration location.

Exit **0** means evidence collection succeeded (conflicts may remain); **1** means
at least one requested probe failed, mismatched or was unsupported; **2** means
invalid input or collection failure. `-o` accepts only a new `out/**/*.json` file
and refuses overwrite. Keep referenced probe workspaces with the report.

## Resolving the blocker

Review original consumers and the full range before accepting a representation.
If the candidate is justified, apply the minimal target-owned change through the
[matching contract](matching.md), regenerate Splat and run applicable native gates.
Then run `bin/harness analysis index --recover` and `just index`; only their actual
success satisfies index freshness. This command never waives that prerequisite
or resumes a blocked campaign.

Implementation: `harness.analysis.boundary` collects evidence,
`harness.analysis.probe` owns isolated experiments, and
`harness.commands.boundary` adapts the canonical CLI.

## Evidence produced with this command

`bin/harness analysis boundary` supplied the ranges and layout facts for
`out/text-format/boundary-resolution.md` (change and preservation checks) and
`out/text-format/boundary-classification.md` (whole-range classification of every
byte, with the consuming source expressions and the range's limits).
