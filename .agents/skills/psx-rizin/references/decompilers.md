# rz-ghidra and r2ghidra

The plugins integrate Ghidra's decompiler/Sleigh with their respective engine;
they are not standalone Ghidra projects and are not interchangeable.

## Detection and compatibility

Record engine version first, then probe inside the engine:

```text
pdg?
pdgs
```

For rz-ghidra, stable plugin releases must match compatible stable Rizin
releases; development tracks development. Do not assume equal version numbers
mean compatibility. The official repository uses `rz-X.Y.Z` tags for exact
Rizin compatibility. Ensure the Sleigh language files were installed.

For r2ghidra, use the plugin's official installation/update path and verify it
inside the exact radare2 version. A common official path is:

```sh
r2pm -U
r2pm -ci r2ghidra
```

Installing dependencies or plugins changes external/local tool state and may
require user approval. Missing decompiler support does not block disassembly,
xrefs, naming, or type evidence work.

## Establish function context first

Before decompiling:

1. Verify raw mapping, MIPS32 little-endian settings, and target identity.
2. Define/correct the function and basic blocks.
3. Inspect direct calls, delay slots, xrefs, and data access widths.
4. Apply only reviewed signatures/types.
5. Decompile the selected function, not the whole binary by default.

## Common commands

```text
pdg                 # decompile current function
pdgj                # JSON result
pdgo                # decompiled lines with offsets
pdgx                # XML output in rz-ghidra
pdgd                # debug XML in rz-ghidra
pdga                # side-by-side output in r2ghidra
pdg*                # emit comments back into analyzer state
pdgs                # list/inspect Sleigh language choices
```

Probe each command; surfaces differ. Avoid `pdg*` during exploratory work
because it mutates comments and can pollute reviewed replay.

## Language selection

Let the plugin auto-select first, inspect `pdgs`, and override only with evidence.
Rizin uses settings such as `ghidra.lang`/`ghidra.sleighhome`; r2ghidra uses
`r2ghidra.lang`, `r2ghidra.timeout`, and related plugin settings. Names evolve,
so query the active plugin configuration rather than copying settings blindly.

For PSX, confirm that the chosen Sleigh language matches 32-bit little-endian
MIPS and produces correct register/instruction semantics at known code. A
plausible pseudocode listing is not sufficient validation.

## Acceptance rules

- Treat pseudocode, local variable names, inferred prototypes, stack layouts,
  unions, switch recovery, and casts as hypotheses.
- Verify MIPS delay slots, sign/zero extension, access widths, register-passed
  arguments, return registers, and branch targets in disassembly.
- Do not copy decompiler text directly into a matching source file and claim it
  compiles or matches.
- Do not promote a function/type/PsyQ name from pseudocode alone.
- Keep decompiler output generated; promote only independently reviewed facts.

## Official sources

- rz-ghidra repository and command/configuration reference:
  https://github.com/rizinorg/rz-ghidra
- r2ghidra repository and command/configuration reference:
  https://github.com/radareorg/r2ghidra
