# Proposal

## Why

`just check` stops at
`tools/rust/bof3-text/tests/text_window_registry.rs::a_freshly_built_registry_is_byte_identical_to_the_retained_one`
because the classified-window registry no longer rebuilds:

```
error: config/targets/emi: reviewed ranges missing for ["ETC/COMMU00.EMI#0"]
```

`reviewed_windows` in `tools/rust/bof3-text/src/cli.rs` derives the registry's reviewed
windows from every `splat.yaml` under `config/targets/emi` (segments of kind `textbin`),
then requires a hard-coded set — `ETC/COMMU00.EMI#0`, `SCENARIO/SCENA00.EMI#0`,
`WORLD00/AREA026.EMI#13` — to be present. Its own comment states the intent: *"Every
reviewed range the boundary record names must be present, so losing one is a failure
rather than a smaller registry."*

Three independent pieces of evidence say the COMMU00 window should exist:

1. `config/targets/emi/etc/commu00/00/splat.yaml` declares a `textbin` entry at offset
   `14588`, bounded by the next entry at `15696` (length **1108**), named `T_801F24FC` —
   and `14588 + 0x801eec00 = 0x801f24fc` matches that record's own `load_address`.
2. The retained registry `out/text-windows/registry.json` already contains exactly that
   window: `{archive: ETC/COMMU00.EMI, entry: 0, start: 14588, length: 1108, kind:
   reviewed_textbin}` — one of only three `reviewed_textbin` windows in the artifact.
3. Narrowing the build to `--targets config/targets/emi/etc/commu00` alone still reports
   it missing, so no sibling record is interfering.

What differs from a working sibling is the record's **shape**: `commu00`'s `splat.yaml`
nests its entries inside mapping-style segments with `subsegments:` (5 top-level entries),
while `scenario/scena00/00/splat.yaml` exposes 48 flat top-level segments. The reader in
`splat_segments` is a lenient line scanner (`- - <offset>` at any indentation), not a YAML
reader, so its view of the two records differs even though both look correct to a YAML
reader and to inspection.

## What Changes

- Pin the exact reason `splat_segments` yields no `textbin` segment for
  `config/targets/emi/etc/commu00/00/splat.yaml`, by dumping what that reader returns for
  the file.
- Apply the narrower of two evidence-supported remedies:
  - **(a) record-side** — normalise `commu00`'s `splat.yaml` so the reader sees the reviewed
    entry, following the sibling convention that already works; or
  - **(b) reader-side** — make `splat_segments` descend into mapping-style `subsegments`,
    which changes harness behaviour and therefore takes the `docs/agents/harness.md`
    ownership note first.
- Restore the reviewed window so the registry rebuilds carrying the window the retained
  artifact already expects (entry 0, start `14588`, length `1108`).
- **If** the record's content changes, regenerate the retained registry so the fresh build
  and the retained artifact agree. That touches **`out/text-windows/registry.json`, a
  disposable `out/` artifact** — called out here because this repository's rules require it.
- No acceptance rule, threshold or evidence gate is relaxed, and the expected-window list is
  not shortened.

## Capabilities

### New Capabilities
- `reviewed-window-registry`: the contract that every reviewed `textbin` range a target
  record declares reaches the classified-window registry, and that a freshly built registry
  matches the retained one.

### Modified Capabilities
(none — no existing requirement covers the reviewed-window registry)

## Impact

- `config/targets/emi/etc/commu00/00/splat.yaml` (remedy a) and/or
  `tools/rust/bof3-text/src/cli.rs` (`splat_segments`, remedy b).
- `out/text-windows/registry.json` — disposable `out/` state, regenerated only if the
  record's hash changes.
- Verification: `cargo test --test text_window_registry` exits 0 and `just check` advances
  past `justfile` line 52.
- Untouched: the other two expected records, the `RETIRED`/`HISTORICAL` guards, and
  `source symbols check`'s pre-existing 89 findings (a further stop at line 67).
