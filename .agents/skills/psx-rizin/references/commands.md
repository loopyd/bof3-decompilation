# Rizin and radare2 command guide

This reference targets current Rizin and radare2. Probe the installed version
and native help before relying on a spelling. Rizin and radare2 share ancestry,
not a stable command-level compatibility contract. Prefer repository adapters
and JSON output in automation, while using the native engine for focused work
the adapter does not expose.

## Capability detection

```sh
rizin -V
r2 -v
rizin -a?
r2 -a?
```

Inside the selected engine:

```text
e asm.arch=??       # architectures
e asm.cpu=?         # CPUs accepted by selected architecture
a?                  # analysis surface
af?                 # functions
ax?                 # references
t?                  # types
P?                  # projects
pdg?                 # Ghidra plugin, if installed
```

Do not infer plugin availability from an executable named `rz-ghidra` or
`r2ghidra`; probe `pdg?` inside the engine.

## Raw PSX opening

```sh
rizin -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD RAW.bin
r2    -N -n -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD RAW.bin
```

Use `-m` to map raw/unknown-header bytes. `-B` overrides a base address for a
recognized binary format and is not the raw-blob mapping switch. On modern
radare2, `-N` avoids user configuration and `-n` skips RBin metadata loading for
the explicitly configured raw blob, preventing noisy/incorrect magic parsing.
Check
`references/psx-inputs.md` before selecting the input or address.

## Navigation and inspection

| Task | Command family | Notes |
| --- | --- | --- |
| Seek | `s ADDRESS`, `s+`, `s-` | `s` without args reports current address |
| Hexdump | `px LENGTH @ ADDRESS` | Verify raw bytes before analysis claims |
| Disassemble | `pd N`, `pdf` | `pdf` requires a function |
| Opcode JSON | `aoj N` | Probe availability/schema per version |
| Describe address | `fd ADDRESS` | Resolves flags around an address |
| Maps/sections | `om`, `iS`, `ij` | Raw input may have no sections |
| Strings | `izz`, `izzj` | Raw strings may require `-zz` or config |
| Print string | `ps @ ADDRESS` | Select `psz`, `psw`, etc. by encoding |

## Staged analysis

Start with `aa` or `aaa`; do not default to the deepest blanket analysis.
`aaaa`/`-AA` may add experimental or expensive stages and false positives.

```text
aa                  # basic analysis
aaa                 # broader function/reference analysis
af @ ADDRESS        # define/analyze function at address
af NAME ADDRESS     # define with name where supported
af+ ADDRESS NAME    # explicit function definition; inspect native help
afu END @ FUNCTION  # resize function to end address
aff @ FUNCTION      # readjust function after edits
afb                 # inspect/manage basic blocks
afl                 # list functions
aflj                # list functions as JSON
```

For overlays or mixed code/data, bound analysis instead of enabling non-code
analysis globally. Inspect engine/version-specific settings using `e analysis.`
(Rizin) or `e anal.`/documented names (radare2). Relevant controls include
analysis range, data references, string references, indirect jumps, jump-table
handling, and maximum function size.

## Names, flags, namespaces, and comments

```text
afn NAME ADDRESS         # rename function
f NAME SIZE @ ADDRESS    # define named flag/data range
fr OLD NEW               # rename flag
f-NAME                   # remove flag
fj                       # flags as JSON
f*                       # replayable flag commands
fs NAME                  # select/create flagspace
fsl                      # list flagspaces (probe on r2 versions)
fsj                      # flagspaces as JSON where supported
CC "reviewed comment" @ ADDRESS
```

Use distinct flagspaces for reviewed functions, reviewed data, strings, PsyQ
correlations, and temporary hypotheses when native support is useful. Never
export a temporary hypothesis as reviewed replay.

## Xrefs and call graphs

```text
axt ADDRESS          # references to address
axf ADDRESS          # references from address
axj                  # JSON xrefs on versions that expose it
axlj                 # JSON xref list on versions that expose it
axffj @ FUNCTION     # function references on supporting r2 versions
agf                  # current function graph
agfj                 # graph JSON where supported
agt                  # call graph in current Rizin command surface
```

JSON spellings vary. Probe the command and validate its returned schema before
putting it into automation. A call target in MIPS `jal` proves an address edge,
not a semantic library identity.

## Search

Bound search ranges and use 4-byte alignment for instruction words when useful:

```text
/ string             # text search
/x HEXPAIRS          # byte pattern
/v4 0xVALUE          # 32-bit value using target endianness
/z MIN MAX           # strings by length
```

Inspect `search.in`, `search.from`, `search.to`, and `search.align` (names can
vary by engine/version). Search hits are observations; confirm whether each hit
is code, data, a pointer, or an accidental byte sequence.

## Types and calling conventions

```text
td "struct Foo { uint32_t value; };"  # define C type
to path/to/types.h                     # import C header
t / tj / t* / tc                       # list, JSON, replay, C output
ts Foo                                 # show structure
tp Foo ADDRESS                         # temporary typed print
avga NAME Foo @ ADDRESS                # modern Rizin typed global
avg                                    # list typed globals
aat [FUNCTION]                         # propagate structure offsets
afc                                    # inspect/set function calling convention
e analysis.cc                          # inspect Rizin default convention
```

`tl TYPE = ADDRESS` exists in older/shared command surfaces and is used by some
repository replay today, but modern Rizin documents `avga` for persistent typed
globals. Treat `tl` as adapter-owned/version-sensitive; never mass-convert replay
without testing both the selected engine and deterministic export.

Use fixed-width integer types for PSX layouts. Verify pointer width, field
offsets, signedness, and the MIPS calling convention against instructions.

## Headless and scripting

```sh
rizin -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD \
  -i reviewed.r2 -c 'aflj' RAW.bin
r2 -N -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD \
  -i reviewed.r2 -c 'aflj' RAW.bin
```

Use no-user-config mode where the engine supports it, disable color, capture
tool/plugin versions, prefer `j` JSON commands or `*` replay commands, and sort
exported records by stable keys. `rzpipe`/`r2pipe` expose `cmd()` and parsed
`cmdj()`; validate schemas and errors rather than assuming all `j` output is
identical across releases.

## Official sources

- Rizin command line: https://book.rizin.re/src/first_steps/commandline_options.html
- Rizin configuration: https://book.rizin.re/src/configuration/evars.html
- Rizin analysis: https://book.rizin.re/src/analysis/code_analysis.html
- Rizin flags: https://book.rizin.re/src/basic_commands/flags.html
- Rizin types: https://book.rizin.re/src/analysis/types.html
- Rizin calling conventions: https://book.rizin.re/src/analysis/calling_conventions.html
- Rizin scripting: https://book.rizin.re/src/scripting/rz-pipe.html
- radare2 command line: https://book.rada.re/first_steps/commandline_flags.html
- radare2 analysis: https://book.rada.re/analysis/code_analysis.html
- radare2 flags: https://book.rada.re/commandline/flags.html
- radare2 types: https://book.rada.re/analysis/types.html
- radare2 syntax: https://book.rada.re/first_steps/syntax.html
- radare2 r2pipe: https://book.rada.re/scripting/r2pipe.html
