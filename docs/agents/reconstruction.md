# Continue reconstruction with an attempt ledger

Native `run`, `capture-supplied` and `verify` commands share one cumulative mission
ledger. `prepare`, ledger `status` and retained `report` inspection need no native
allowance. Discover arguments with `bin/harness reconstruction --help` and each
command's `--help`.

## Recover history and authorize continuation

Initialize from a verified preparation containing the frozen 118-member corpus
and seven exclusions. The ledger directory must match the mission name.

```sh
ledger=out/retdec-118-reconstruction/attempt-ledgers/my-mission
bin/harness reconstruction ledger init "$ledger" \
    --mission my-mission --prepared-run "$prepared_run"
bin/harness reconstruction ledger status "$ledger"
```

Retain the returned `digest` outside the ledger. Every mutation requires that
value through `--expected-ledger-digest` and returns the new digest. Concurrent
writers, stale digests, missing records and altered records cause refusal.

Import each audited historical operation with its evidence pin and operation units.

```sh
bin/harness reconstruction ledger import "$ledger" \
    --expected-ledger-digest "$ledger_digest" \
    --operation-id prior-run --units "$consumed_units" \
    --evidence "$evidence" --expected-evidence-sha256 "$evidence_sha256"
```

Import one operation once. Byte-identical report copies are not additional attempts.
An identical repeated import is harmless; a conflicting ID or reused evidence hash
causes refusal. Zero-unit imports can retain preparation publications without
charging a native operation.

Historical consumption remains a minimum and `history_complete` stays false.
Missing history does not become zero consumption or a recovered remaining budget.
Recapture must retain the same members, target identities and original image bytes.
Existing output-reuse guards still require unchanged execution inputs.

Record a separately authorized finite allowance for the exact selected members.

```sh
bin/harness reconstruction ledger grant "$ledger" \
    --expected-ledger-digest "$ledger_digest" \
    --grant-id continuation-1 --units "$authorized_units" \
    --member exe/slus_004_22@8014b17c
```

Grants do not refund old consumption. Execution uses the latest grant; unused
capacity from older grants does not carry into it. Each grant permits 1 to 4,250
units. Native commands require `--attempt-ledger "$ledger"` and
`--expected-ledger-digest "$ledger_digest"`, plus their other required arguments.
Use a fresh destination for every execution.

The owner reserves units durably before each operation, including failed calls.
Preparation and self-check cost one unit each. Native generation, syntax, each
compiler profile and associated verification cost one unit each. Independent
rebuild costs six units and comparison costs five units. Exhaustion stops the
next owner call. Source-form search also retains its member and campaign limits.

## Recover an interrupted execution

An interrupt or uncertain native cleanup leaves an unfinished execution in
`ledger status`, blocking further native work. Poll the original session handle
and establish that the complete writer tree has stopped. A free lease, stored PID
or elapsed time does not establish termination.

After independent terminal-writer review, retain its evidence and SHA-256, then
record the operator's attestation.

```sh
bin/harness reconstruction ledger close "$ledger" \
    --expected-ledger-digest "$ledger_digest" --epoch "$execution_destination" \
    --writer-tree-stopped --evidence "$review" \
    --expected-evidence-sha256 "$review_sha256"
```

`close` records the review pin and current report or latest progress checkpoint.
It does not stop processes or refund reservations. Source-form resume accepts
only a publication pinned by the latest closed run, subject to its existing
complete input and checkpoint validation. Closed accounting proves no byte
fidelity, semantic acceptance or whole-corpus reconstruction.
