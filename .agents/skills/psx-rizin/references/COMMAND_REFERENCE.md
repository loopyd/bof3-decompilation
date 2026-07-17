# Command reference

## Bundled tools

```bash
# Inspect header
python3 scripts/psx_exe.py inspect GAME.EXE

# JSON output
python3 scripts/psx_exe.py inspect GAME.EXE --json

# Extract payload
python3 scripts/psx_exe.py extract GAME.EXE -o GAME.payload.bin

# File/runtime conversion
python3 scripts/psx_exe.py offset-to-addr GAME.EXE 0x1234
python3 scripts/psx_exe.py addr-to-offset GAME.EXE 0x80010a34

# Raw MIPS triage
python3 scripts/scan_mips.py OVERLAY.BIN --base 0x80180000 --json scan.json

# Rizin inventory
python3 scripts/rizin_export.py GAME.payload.bin --base 0x80010000 --out out/inventory

# Per-function artifact bundle
python3 scripts/function_artifacts.py GAME.payload.bin 0x80012340 \
  --base 0x80010000 --out out/functions/80012340

# Convert reviewed symbols
python3 scripts/symbols_to_rizin.py symbols.csv -o imported-symbols.rz

# Replay coverage
python3 scripts/replay_coverage.py replay-matrix.csv --report replay-report.md
```

## Rizin essentials

```text
? / <cmd>?               help
s <address>              seek
pd <instructions>        disassemble instruction count
pD <bytes>               disassemble byte count
pdf                      function disassembly
px <bytes>               hex
izj                      strings JSON
aflj                     functions JSON
axlj                     xrefs JSON
```

## Analysis

```text
aa                       baseline roots
aaa                      advanced auto-analysis; inspect results
aab                      basic-block analysis
aaf                      all function calls
aac                      calls from selected/current function
aar                      data references
aad                      pointers-to-pointers
af                       analyze function
afu <end>                resize function
```

## Xrefs

```text
axt / axtj               to current address
axf / axfj               from current address
axg                      reachability graph
axC <target>             add call xref from seek
axc <target>             add code xref
axd <target>             add data xref
axs <target>             add string xref
```

## Variables/types

```text
afvl                     list variables/arguments
afva                     analyze variables/arguments
afv=                     variable accesses
afs                      function signature
td                       define C type
to                       load C header
ts                       structs
tp                       print typed data
```

## rz-ghidra

```text
pdg                      decompile
pdgo                     offsets + decompile
pdgj                     JSON
pdgx                     XML
pdgs                     languages
```

## Headless shell pattern

```bash
rizin -q -a mips -b 32 -e cfg.bigendian=false -m 0x80010000 \
  -c 'aa;aar;aaf;aflj;q' payload.bin
```

For production scripts, prefer one JSON command per controlled invocation or `rzpipe`; store stderr separately.

## Runtime starting commands

PCSX-Redux GDB examples in its documentation use an interpreter/debugger/GDB startup and a client connecting to `localhost:3333`. Confirm current flags in installed PCSX-Redux.

```bash
gdb-multiarch
(gdb) set architecture mips
(gdb) target remote localhost:3333
```

Record emulator CLI/settings in the case rather than relying on GUI state.
