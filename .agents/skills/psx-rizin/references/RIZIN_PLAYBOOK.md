# Rizin playbook for PS1 binaries

> In the BOF3 repository, prefer the wired `bin/rz-project` wrapper:
> `bin/rz-project analyze TARGET`, `bin/rz-project status TARGET`,
> `bin/rz-project open TARGET`. The generic `rizin` invocations below are the
> skill-local fallback for work outside the repo's `out/reverse/<target>/`
> workspace.

## Canonical raw mapping

Extract a PS-X EXE payload or identify a raw overlay base, then open:

```bash
rizin -a mips -b 32 \
  -e cfg.bigendian=false \
  -m 0x80010000 \
  -i assets/rizin/psx-init.rz \
  payload.bin
```

Use `-m` to map a raw file at the proven runtime address. Confirm the installed Rizin CLI with `rizin -h`; option behavior can evolve.

## Staged analysis

Recommended progression:

```text
aa      entrypoints/symbol roots
aar     data references
aaf     call targets
aac     calls from focused functions
aad     pointers-to-pointers references
```

Use `aaa` only after checking ranges and code/data boundaries. The Rizin handbook explicitly warns fully automated analysis can produce nonsensical results and exposes individual phases for control.

Useful configuration:

```text
e analysis.hasnext=false
e analysis.jmp.indir=true
e analysis.jmp.tbl=true
e analysis.datarefs=true
e analysis.refstr=true
e analysis.strings=true
```

For raw mixed files, set analysis ranges where possible. Inspect `e analysis.in=??` on the installed version.

## Navigation and display

```text
s <addr>               seek
pdf @ <func>            disassemble function
pd 20 @ <addr>          20 instructions
pD 80 @ <addr>          80 bytes interpreted as disassembly
px 64 @ <addr>          64-byte hex dump
iz / izz / izj / izzj   strings; use `izzj` JSON for raw images
```

`pd` counts instructions; `pD` counts bytes.

## Functions

```text
af @ <addr>             analyze function
afu <end> @ <start>     resize/reanalyze through end
af+ <name> ...          handcraft when needed
afb @ <addr>            list basic blocks
afi / afij              function information
afn <name> @ <addr>     rename function
afs                     show/edit signature (check `afs?`)
```

Do not use a plausible prologue alone as proof. Direct/runtime edges are stronger.

## Calls and xrefs

```text
axt @ <addr>            xrefs to address
axtj @ <addr>           JSON xrefs to
axf @ <addr>            xrefs from address
axfj @ <addr>           JSON xrefs from
axg @ <addr>            graph paths reaching target
axl / axlj              all xrefs
axC <target> @ <from>   add call xref
axc <target> @ <from>   add generic code xref
axd <target> @ <from>   add data xref
axs <target> @ <from>   add string xref
```

Add manual xrefs only after recording how the target was derived.

## Variables and arguments

```text
afvl @ <func>           list function variables/arguments
afva @ <func>           analyze arguments/locals
afv= @ <func>           show variable accesses
```

Use installed help (`afv?`, `afc?`, `afs?`) before scripted mutations. Argument recovery is a caller/callee/runtime exercise; the command output is only one source.

## Analysis hints

When control flow is wrong:

```text
ahc <target> @ <addr>    override jump/call target
ahd <opcode> @ <addr>    override displayed opcode
ahs <size> @ <addr>      override opcode size
ahl                      list hints
```

Hints are preferable to globally increasing analysis when one instruction/table is the actual issue.

## GP and jump tables

```text
e analysis.gp=<value>
e analysis.gpfixed=true|false
```

Use cautiously. The Rizin handbook notes MIPS GP can change by function and these controls are experimental.

Jump-table support is influenced by:

```text
e analysis.jmp.tbl=true
e analysis.jmp.indir=true
e analysis.datarefs=true
```

Manual reconstruction remains necessary for overlay-relative, relocated, compressed, or script dispatch tables.

## Flags, comments, and namespaces

```text
f name @ <addr>         create flag
fr old new              rename flag
fs symbols              select/create flagspace
fC name comment         flag comment
```

Use namespaces such as:

```text
main.*
ovl.<id>.*
psyq.*
bios.*
data.*
trace.*
```

Do not erase original symbol spelling. Add normalized aliases.

## Types

Core type-oriented commands include:

```text
td <declaration>        define C type
to <header>             load C header
ts                      list/show structs
tp <type> @ <addr>      print memory as type
```

Consult `t?`, `afc?`, and variable/type help for the installed release. Apply types after the offset ledger is coherent, then verify that type propagation did not hide contradictory machine-code behavior.

## Decompiler

With compatible rz-ghidra:

```text
pdg                     decompile current function
pdgo                    decompile side-by-side with offsets
pdgj                    JSON decompiler output
pdgx                    XML output
pdg*                    return decompilation as Rizin comments
pdgs                    list loaded Sleigh languages
```

Pin rz-ghidra to a tag compatible with the installed Rizin release. Compare `pdg` against `pdf` and runtime observations.

## Signatures

Rizin's FLIRT workflows use the `F` command family and `rz-sign`. Typical tasks are creating patterns/signatures from symbolized libraries, loading a signature database, and applying matches. Check `F?`/`rz-sign -h` because paths and exact subcommands can change.

## JSON and automation

Prefer JSON outputs for durable tooling:

```text
ij       core/binary information
aflj     functions
axlj     xrefs
izzj     raw strings
afij     current/function info
axtj     xrefs to
axfj     xrefs from
pdgj     decompiler JSON
```

The bundled scripts use independent Rizin invocations to reduce interactive-state ambiguity. For high-volume work, use `rzpipe` and retain command logs plus raw JSON.

## Function artifact minimum

A function directory should contain:

```text
metadata.json
disassembly.txt
decompile.txt
decompile-offsets.txt
decompile.json
xrefs-to.json
xrefs-from.json
variables.txt
notes.md
```

Regenerate artifacts after boundary, symbol, type, or xref changes and diff the directories.
