---
name: harness
description: Use the BOF3 harness (bin/harness) for function targeting, claiming, lifting, and binary mapping. Use when the user asks about targets, candidates, claims, or the harness workflow. Also use for module-level verification and binary parity.
---

## Entry
`bin/harness` with subcommands: `status`, `setup`, `catalog`, `analyze`, `candidates`, `claim`, `lift`, `verify`, `finish`, `report`, `dashboard`, `resume`.

## Function Loop
```bash
bin/harness setup && bin/harness analyze
bin/harness candidates --module emi:ETC/GAME#0 --min-size 32 --limit 10
bin/harness claim --type function --owner "$USER"
bin/harness lift <target-id>
bin/harness verify function <source>
bin/harness finish <target-id> --status done --message "matched"
```

## Module Verification
```bash
bin/harness verify module emi:ETC/SHOP#0 --allow-different
bin/harness verify binary emi:BATTLE/BATTLE#3
```

## Target IDs
- `func:<archive>#<slot>@<address>` e.g. `func:ETC/GAME#0@0x801ba678`
- `emi:<archive>#<slot>` e.g. `emi:BATTLE/BATTLE#3`

## Lifecycle
`make decomp-full-ready` (full Ghidra refresh) or `make lift-ready` (fast harness refresh).

Ghidra headless writes go through `bin/harness ghidra import-project` or `bin/harness ghidra export-symbols`.
