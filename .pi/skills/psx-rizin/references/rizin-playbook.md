# Rizin playbook for PS1 binaries

> In this repo prefer `bin/harness analysis rz-project analyze TARGET`, `bin/harness analysis rz-project status TARGET`, and `bin/harness analysis rz-project open TARGET` (writes `out/reverse/snapshots/<encoded-target>.json`). Outside this workspace use the generic fallback below. Generic command lookup: [command-reference.md](command-reference.md).

## Canonical raw mapping

Extract a PS-X EXE payload or prove a raw overlay base, then:

```bash
rizin -a mips -b 32 \
  -e cfg.bigendian=false \
  -m 0x80010000 \
  -i assets/rizin/psx-init.rz \
  payload.bin
```

`-m` maps raw data at its proven runtime address. Confirm the installed CLI with `rizin -h`; options evolve.

## Staged analysis

Order: `aa` (roots) → `aar` (data refs) → `aaf` (call targets) → `aac` (calls from focus) → `aad` (ptr-to-ptr). Use `aaa` only after checking ranges/boundaries; full automation can be nonsensical.

```text
e analysis.hasnext=false
e analysis.jmp.indir=true
e analysis.jmp.tbl=true
e analysis.datarefs=true
e analysis.refstr=true
e analysis.strings=true
```

For mixed raw files, set analysis ranges where possible; inspect `e analysis.in=??`.

## Review gates

| Area | Gate |
|---|---|
| Functions | A plausible prologue alone is not proof; direct/runtime edges are stronger. |
| Calls/xrefs | Before adding a manual xref, record how its target was derived. |
| Variables/arguments | Check `afv?`, `afc?`, `afs?` before scripted mutations. Recover arguments from callers, callees, and runtime; command output is only one source. |
| Hints | Prefer local hints over global analysis when one instruction/table is wrong. |
| GP/jump tables | `analysis.gp`/`analysis.gpfixed` are experimental; GP may vary by function. Manually reconstruct overlay-relative, relocated, compressed, or script dispatch tables. |
| Flags/namespaces | Preserve original spelling; add normalized aliases. Namespaces: `main.*`, `ovl.<id>.*`, `psyq.*`, `bios.*`, `data.*`, `trace.*`. |
| Types | Apply only after the offset ledger is coherent; reject propagation that hides contradictory machine-code behavior. |
| Interpretation | Reconcile Rizin disassembly, callers, xrefs and runtime; mark proposed C as hypothesis until corroborated. |
| Signatures | Create patterns from symbolized libraries → load → apply. Record false positives and functions too short for safe identification. |
| Automation | Prefer durable JSON; use independent invocations to reduce interactive-state ambiguity, or for volume `rzpipe`, command logs, and raw JSON. |

## Function artifact minimum

```text
metadata.json
disassembly.txt
xrefs-to.json
xrefs-from.json
variables.txt
notes.md
```

Regenerate and diff the directory after boundary, symbol, type, or xref changes.
