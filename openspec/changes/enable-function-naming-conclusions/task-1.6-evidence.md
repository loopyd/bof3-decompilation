# Task 1.6 — canonical selected-call journal proof

## Result

Proved on `emi/world04/area191/13`, row `function:func_801F2E34`.
No production code, tests, identity files or gates changed. Evidence is retained
under `out/reviews/evidence/task-1.6-canonical/` (ignored, local-only).
Independent review remains required; this does not establish conclusion admission.

The unedited tool-generated `area191-report.json` contains three open
`caller_context` work items: `caller:emi/world04/area191/13@801f2d00`,
`caller:emi/world04/area191/13@801f2dbc`, and
`caller:emi/world04/area191/13@801f2e14`. The last is a fully decoded 32-byte
straight-line wrapper calling the local selected function. Scouting used the
existing SQLite calls/functions tables and original-byte decoder read-only.
Earlier canonical initializations for battle/03 and scena00/00 are retained;
the candidate callees there had no raw-name inventory row and were not run.

## Exact canonical commands

```sh
bin/harness naming init emi/battle/battle/03 out/reviews/evidence/task-1.6-canonical/report.json
bin/harness naming init emi/scenario/scena00/00 out/reviews/evidence/task-1.6-canonical/scena00-report.json
bin/harness naming init emi/world04/area191/13 out/reviews/evidence/task-1.6-canonical/area191-report.json
bin/harness naming evidence emi/world04/area191/13 out/reviews/evidence/task-1.6-canonical/area191-report.json --rows function:func_801F2E34 --instructions --evidence-root /mnt/PROJECTS/repos/rebof3-simple/out/reviews/evidence/task-1.6-canonical
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=tools/python python out/reviews/evidence/task-1.6-canonical/verify.py
just check-unit naming
just check
```

All initializations and collection exited 0. Collection executed one row,
resumed zero, reported no errors, and did not terminalize. Both gates were
wrapped with `strace -f -qq -yy -e trace=openat,creat,rename,renameat,renameat2,unlink,unlinkat,write,pwrite64,truncate,ftruncate -o PATH`
to retain mutation-path evidence. Logs, exit files and traces use `check-unit`
and `check` stems. The verification command was repeated after both gates.

## Operation agreement and freshness

Namespace: `emi__world04__area191__13__2d4d6c7a80d1/` beneath the evidence root.
`verification.json` and `verification-after-gates.json` independently reconstruct
current report digest, index SHA-256, target-manifest hashes and operation digest
using the same inputs as the canonical runner, then invoke the unchanged
`journal_is_fresh`. Neither uses recorded manifest values as expected inputs.
Both report `journal_is_fresh: true` and `operations_agree: true`.
`verify.py` is an inspection script, not a regression test or artifact producer.
It selects the same instruction-capture context as `--instructions`.

Payload (three indexed items plus four semantic captures), checkpoint, and
report-derived plan have these identical **ordered** identities:

```text
indexed:caller:emi/world04/area191/13@801f2d00:calls:emi/world04/area191/13@801f2d00:0
indexed:caller:emi/world04/area191/13@801f2dbc:calls:emi/world04/area191/13@801f2dbc:0
indexed:caller:emi/world04/area191/13@801f2e14:calls:emi/world04/area191/13@801f2e14:0
semantic:instructions-v1:emi/world04/area191/13@801f2d00
semantic:instructions-v1:emi/world04/area191/13@801f2dbc
semantic:instructions-v1:emi/world04/area191/13@801f2e14
semantic:instructions-v1:emi/world04/area191/13@801f2e34
```

No payload, receipt, report, checkpoint or manifest was hand-edited. Digest and
operation comparisons, capability admission and unsupported-fact rejection are
unchanged; protected-file PRE/POST comparison includes the naming implementation.

## Fact and retained pins

`func_801F2E34/derived.json` contains one positive `selected_call`:
caller `emi/world04/area191/13@801f2e14`, callsite `0x801F2E1C` word
`0x0C07CB8D`, callee `0x801F2E34`, delay slot `0x801F2E20` word `0x00000000`;
a0–a3 retain entry registers 4–7. Guards are absent on this fully decoded
straight-line wrapper, v0/v1 are forwarded uninterpreted, and the return restores
24 stack bytes. The receipt retains complete native output for
`bin/harness analysis rz-project query emi/world04/area191/13 -c 'pdj 8 @ 0x801F2E14 @!32'`
and the typed observation.

| Artifact (namespace-relative) | SHA-256 |
|---|---|
| `func_801F2E34/semantic-2-a6bc675a0b0a.json` | `300d514117fc7947c94f6d2081d4fb3b4de091fc02c156242371984bdeedddf9` |
| `func_801F2E34/payload.json` | `3d7d426098a296c46c7470e977973bf3e91728bc2d74e36a23ef9976ab7c15b3` |
| `func_801F2E34/derived.json` | `d04dac536bea3e3a7e0220cfa889c5ffcd7ecef47f8564f1a5534a36da80c1c5` |
| `manifest.json` | `0922b144c1839db511e294db6d55b90876f80e7074caf700406c5c24cbec9f28` |

Fact source ID: `original-instructions:a00cc332478c01b2971db2bbd6dd13a433ac06ee7eca2414a21617fe06d2354b`.

## Required checks and writes

- `just check-unit naming`: **exit 2**. Naming pytest: **652 passed, 2 skipped in
  37.86s**. Its chained `just check` fails at `source symbols check` (line 67),
  including 14 binding/map drifts and 61 raw-spelling debts. First diagnostic:
  `src/bof3/support/area000_13_psyq.c` / `func_801E5988` / `0x801E5988`.
- `just check`: **exit 2**, same symbol gate and stopping point. Ruff passes,
  docs drift is zero, Rust text tests and release build pass, text corpus gates
  pass, Python text tests **32 passed in 3.86s**. `validate_sources` is unreached.
- The two naming skips are parameterizations of
  `test_rejects_reviewed_battle15_unsound_conclusions`: disposable forensic inputs
  `function_func_8009704C.json` and `data_D_80096994.json` are unavailable.

`gate-written-paths.json` lists every observed gate mutation/writable-open path
and tracked/ignored classification, including transient paths; its extraction
script and raw traces are retained. There are no unresolved paths. Repository
paths are all ignored:

- `build/tools/rust/bof3-text/release/{.cargo-artifact-lock,.cargo-build-lock,.cargo-lock}`
  (writable opens; not proof of content changes);
- `build/tools/rust/bof3-text/release/bof3-text.d`;
- `out/text-payloads/inventory.withheld.json` (transient, removed);
- `out/text-windows/registry.json`;
- `out/reviews/evidence/task-1.6-canonical/{check-unit.log,check.log}`.

The inventory additionally retains 5,280 external paths, principally disposable
pytest/Rust fixtures and external cache/lock paths; these are outside repository
tracking. Successful writable opens are conservatively included. Observer logs,
traces, scripts, verification output, preservation snapshots, initial reports and
canonical runner artifacts all reside under the ignored evidence directory.

`preservation.json` compares 3,834 protected PRE paths: config, BOF3 sources,
naming implementation, dirty justfile and raw Git index. **Zero drift.** Baseline
SHA-256 remains `afe50b04b1445d834d96b7fb16460f19efaa53a32a81d3cea872f5b6e3d12718`.
Raw index SHA-256 remains
`b9ade40c2777161069564372c5d027e573bb4b67a5a835b3c3d50564e8e7e81f`;
staged-diff SHA-256 remains
`8d91725c5a7afb6f44c9da0e6774fec5dedd32a26707e6edc57509137fad3e5d`.
No files were staged or unstaged; the substantial pre-existing staged index is
preserved, not empty. No map, target manifest or support identity was applied.

## Skips and limits

No full `just check-all`, unrelated Python suites, native lift byte-match/build
acceptance, naming conclusion/identity transaction, full-target audit validation,
or new regression tests were run. `validate_sources` was blocked by the required
recipe's symbol gate, not silently bypassed. Hindsight extension tools were not
available in this worker's allowlist.

This proves task 1.6 only. Function conclusions remain disabled; selected range
and one-level-beyond remain open, and other captured functions have explicit
selected-call decode refusals. Imported caller discovery, owner-body production,
branching coverage and campaign unblocking are not proved. Evidence is local and
ignored and must be retained for review; freshness depends on current inputs and
the same instruction mode. Checks do not establish a clean repository, and
filesystem observations do not exclude concurrent external writers.
