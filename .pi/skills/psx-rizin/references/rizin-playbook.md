# Rizin playbook for PS1 binaries

Use `bin/harness analysis rz-project analyze TARGET` to write
`out/reverse/snapshots/<encoded-target>.json`.
`bin/harness analysis rz-project status TARGET` checks it, and
`bin/harness analysis rz-project open TARGET` loads the reviewed replay.
Outside this workspace, use the raw mapping below. Command lookup:
[command-reference.md](command-reference.md).

## Canonical raw mapping

Extract a PS-X EXE payload or prove a raw overlay base, then:

```bash
rizin -N -n -a mips -b 32 -E little \
  -m 0x80010000 \
  payload.bin
```

`-m` maps raw data at its proven runtime address. Confirm the installed CLI with `rizin -h`; options evolve.

For PS1 decompilation, build the sibling `rz-psxdec` and `retdec-psx` forks through
`bin/harness setup --component rz-psxdec`. Load the printed `rz-psxdec.so`
explicitly with `RZ_NOPLUGINS=1` and `-l`; see the
[workspace guide](../../../../tools/retdec-psx/README.md). Ordinary `pdz` uses Rizin
context. Original-only `pdzar /absolute/workspace` requires the validated fixed
inputs; it does not establish semantic acceptance.

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
