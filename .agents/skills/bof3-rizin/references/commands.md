# Rizin and radare2 commands

## Engine selection

Run `bin/harness analysis doctor`. Prefer `rizin`; otherwise use `r2`. Treat
`rz-ghidra` (Rizin) and `r2ghidra` (radare2) as different plugins.

## Raw PSX input

The required settings are MIPS, 32-bit, little-endian, and the verified runtime
load address:

```sh
rizin -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD RAW_BINARY
r2    -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD RAW_BINARY
```

Use the extracted raw EMI entry or normalized executable load image, never the
EMI archive or PS-X EXE wrapper.

## Harness-normalized command subset

These spellings are verified against radare2 6.1.4. The harness owns Rizin
compatibility and must test differences against the detected version.

| Task | Command |
| --- | --- |
| Analyze | `aaa` |
| List functions as JSON | `aflj` |
| Rename function | `afn NAME ADDRESS` |
| Define flag | `f NAME SIZE @ ADDRESS` |
| Rename flag | `fr OLD NEW` |
| List xrefs as JSON | `axlj` |
| Xrefs to address | `axt ADDRESS` |
| Xrefs from address | `axf ADDRESS` |
| List strings as JSON | `izzj` |
| Import C header | `to PATH` |
| Define C type | `td DECLARATION` |
| Show struct | `ts TYPE` |
| Print typed value | `tp TYPE ADDRESS` |
| Link type to address | `tl TYPE = ADDRESS` |

Prefer JSON commands in scripts and exports. Sort exported records by address
and name so repeated exports are diffable.

## Projects

Project commands vary by engine and release. Current radare2 uses `P+ NAME` or
`Ps NAME` to save, `P NAME` to open, and `r2 -p NAME`; `dir.projects` selects
the generated project root. Rizin project commands are adapter-owned and must
be checked against the detected version.
Use `bin/harness analysis init|export|query` instead of embedding a native
project command in documentation or automation.

## Decompilers

- rz-ghidra is a Rizin plugin and commonly exposes `pdg`/`pdgj`.
- r2ghidra is a radare2 plugin with similar command names but separate state.
- Detect the command before use. A missing plugin is not an analysis failure.
- Decompiler output is a naming and control-flow hint, not source or matching
  authority. rz-ghidra has incomplete union support; verify overlays manually.
