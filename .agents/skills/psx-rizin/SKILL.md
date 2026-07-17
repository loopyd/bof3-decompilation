---
name: psx-rizin
description: Manually invoked, evidence-driven PlayStation 1 reverse-engineering workflow using Rizin, rz-ghidra, emulator replays, symbols, signatures, xrefs, offsets, overlays, and matching decompilation. Use only when the user explicitly invokes `$psx-rizin` or explicitly asks to load the psx-rizin skill by name. Do not auto-trigger for ordinary reverse-engineering questions.
license: MIT
compatibility: Linux, macOS, or WSL; Python 3.10+; Rizin recommended; optional rz-ghidra, Cutter, PCSX-Redux, BizHawk, DuckStation, Ghidra, splat, spimdisasm, maspsx, asm-differ, objdiff, decomp-permuter, and a legally obtained PS1 BIOS/game image.
metadata:
  author: OpenAI
  version: "1.0.0"
  invocation: "$psx-rizin"
  platform: "Sony PlayStation / PS1 / PSX"
---

# PSX Rizin

Use this skill only after explicit invocation. Treat it as an engineering workflow, not a command dump.

## Objective

Build a reproducible, evidence-backed model of a PS1 game from legally obtained binaries and runtime observations. Recover binary layout, load addresses, functions, arguments, types, globals, overlays, call/data xrefs, PsyQ/library identities, replay coverage, and—when requested—matching C.

The source of truth is the original machine code plus repeatable runtime evidence. Decompiler output, guessed symbols, and signatures are hypotheses until corroborated.

## Non-negotiable rules

1. Work only from game images, BIOS images, SDK files, symbols, and manuals the user is authorized to possess and analyze. Never redistribute proprietary game or SDK payloads.
2. Hash every input before analysis. Record region, revision, executable name, BIOS hash, emulator version, and tool versions.
3. Preserve three coordinates for every finding: file offset, runtime virtual address, and overlay/module identity. Never record a bare address without its address space.
4. Treat cached `0x8.......` and uncached `0xA.......` RAM addresses as aliases only after masking and validating the physical address.
5. Respect MIPS branch and load delay slots. Never infer a call argument or branch condition without inspecting the delay-slot instruction.
6. Do not run the deepest automatic analysis blindly over raw mixed code/data. Start staged, inspect errors, then expand.
7. Do not accept a function prototype solely because `pdg` produced one. Validate arguments at callers, inside the callee, and dynamically when practical.
8. The same runtime address may identify different code in different overlays. Keep one analysis namespace/project per overlay instance or load identity.
9. Imported symbols and signatures must carry provenance and confidence. Preserve original names exactly; keep normalized names as aliases.
10. Persist work under `.agent-work/psx-rizin/<case-id>/` unless the repository already defines a canonical reverse-engineering workspace.

## Read references progressively

Read only what is needed for the current phase:

- End-to-end procedure: [references/WORKFLOW.md](references/WORKFLOW.md)
- PS-X EXE, aliases, MIPS ABI, delay slots: [references/PSX_ABI_AND_ADDRESSING.md](references/PSX_ABI_AND_ADDRESSING.md)
- Rizin analysis, xrefs, variables, types, JSON, decompiler: [references/RIZIN_PLAYBOOK.md](references/RIZIN_PLAYBOOK.md)
- Symbols, PsyQ signatures, confidence rules: [references/SYMBOLS_SIGNATURES_AND_TYPES.md](references/SYMBOLS_SIGNATURES_AND_TYPES.md)
- Overlay discovery and mapping: [references/OVERLAYS_AND_ASSETS.md](references/OVERLAYS_AND_ASSETS.md)
- Replays, breakpoints, traces, coverage: [references/RUNTIME_AND_REPLAYS.md](references/RUNTIME_AND_REPLAYS.md)
- Matching decompilation and build diff: [references/DECOMP_BUILD_DIFF.md](references/DECOMP_BUILD_DIFF.md)
- Command recipes: [references/COMMAND_REFERENCE.md](references/COMMAND_REFERENCE.md)
- Manuals and primary/community references: [references/MANUALS_AND_SOURCES.md](references/MANUALS_AND_SOURCES.md)

## Invocation grammar

Interpret explicit calls as one of these modes:

```text
$psx-rizin inventory <disc-or-directory>
$psx-rizin inspect-exe <PS-X-EXE>
$psx-rizin analyze <binary> [base-address]
$psx-rizin analyze-overlays <directory>
$psx-rizin function <binary> <runtime-address> [base-address]
$psx-rizin symbols <symbol-source>
$psx-rizin trace <replay-or-scenario>
$psx-rizin replay-coverage <replay-directory>
$psx-rizin build-diff [function-or-target]
$psx-rizin audit <case-directory>
```

A free-form task after `$psx-rizin` is valid. Resolve it to the nearest mode and document assumptions instead of blocking on minor ambiguity.

## Default workflow

### 1. Create the case

Create:

```text
.agent-work/psx-rizin/<case-id>/
  manifest.yaml
  hashes/
  inventory/
  extracted/
  rizin/
  symbols/
  functions/
  overlays/
  replays/
  traces/
  evidence/
  reports/
```

Copy the templates from `templates/`. Do not copy proprietary binaries into a shareable output directory; symlink or reference local paths where appropriate.

Capture tool versions:

```bash
rizin -v || true
rz-bin -v || true
rz-asm -v || true
python3 --version
```

### 2. Inventory before disassembly

Classify every relevant file:

- CUE/BIN or ISO 9660 disc image
- `SYSTEM.CNF` and boot executable
- PS-X EXE
- raw MIPS overlay/module
- compressed or packed archive
- PsyQ `CPE`, `SYM`, `MAP`, `OBJ`, `LIB`, or ELF
- XA/CD audio, STR/MDEC video, TIM textures, VAB/VAG audio, memory-card/replay/save data
- scripts, bytecode, level data, model/animation formats

For every file record SHA-256, size, magic, source path, extraction method, guessed load address, confidence, and parent container.

Do not assume a `.BIN` file is executable. Use content, loader behavior, and runtime writes.

### 3. Normalize PS-X EXE addressing

Run:

```bash
python3 scripts/psx_exe.py inspect GAME.EXE
python3 scripts/psx_exe.py extract GAME.EXE -o .agent-work/psx-rizin/<case>/extracted/GAME.payload.bin
```

For a normal PS-X EXE payload:

```text
runtime_address = text_load_address + (file_offset - 0x800)
file_offset     = 0x800 + (runtime_address - text_load_address)
```

Validate bounds before using the formula. For raw overlays, use that overlay's independently proven load base and raw file offset zero.

### 4. Establish a staged Rizin baseline

Use the extracted payload for predictable raw mapping:

```bash
bin/lift GAME.EXE
```

Equivalent core invocation:

```bash
rizin -a mips -b 32 \
  -e cfg.bigendian=false \
  -m <text-load-address> \
  -i assets/rizin/psx-init.rz \
  GAME.payload.bin
```

The baseline script performs conservative staged analysis. Inspect function boundaries, data interpreted as code, indirect dispatch, GP-relative references, and executable ranges before increasing analysis depth.

Use `pd N` for N instructions and `pD N` for N bytes. Do not mix the units in evidence notes.

### 5. Recover functions and xrefs

For each candidate function:

1. Prove the entry from direct calls, indirect dispatch, symbol/signature evidence, a runtime execution breakpoint, or a credible prologue/control-flow root.
2. Define or repair the function and basic blocks.
3. Inspect xrefs to and from it using `axt`, `axf`, and reachability using `axg`.
4. For unresolved indirect calls/jumps, reconstruct the table and add manual call/code/data xrefs only with recorded evidence.
5. Inspect the call delay slot and every relevant load delay.
6. Export a function artifact bundle with `scripts/function_artifacts.py`.

Useful Rizin actions include:

```text
af                  analyze function at seek
afu <end>            resize/reanalyze function to end
ac? / aac / aaf      call-oriented analysis
aar / aad             data and pointer reference analysis
axt / axf / axg       xrefs to, from, and reachability graph
axC / axc / axd       manually add call, code, or data xref
ahc / ahd / ahs       analysis hints for wrong targets/opcodes/sizes
```

Always consult the installed version's `?` help before scripting a command that mutates analysis state.

### 6. Recover all plausible arguments

Use the PS1 MIPS calling convention as a starting hypothesis:

- `a0`–`a3`: first four argument words
- additional arguments: caller stack argument area
- `v0`–`v1`: return values
- `s0`–`s7`, `fp`: callee-saved
- `t0`–`t9`: caller-saved temporaries
- `ra`: return address
- `gp`: small-data/global pointer, potentially varying by module or function

For every call site:

1. Backward-slice definitions reaching `a0`–`a3`.
2. Include the call's delay-slot instruction; it may set an argument.
3. Inspect stack stores before the call for argument 5+.
4. Compare all callers; distinguish constants, pointers, handles, enums, sizes, and implicit context.
5. Inspect which incoming registers/stack slots the callee reads before overwrite.
6. Break at function entry in PCSX-Redux and capture registers plus the relevant stack window across multiple replays.
7. Record a minimum, maximum, and semantic prototype confidence rather than forcing one signature early.

Use `afvl`, `afva`, `afv=`, function signatures, and user-defined types to represent conclusions only after evidence is sufficient.

### 7. Recover structs, globals, and offsets

Build an offset ledger for repeated accesses such as `lw t0, 0x34(a0)`:

- base register and inferred object role
- signed decimal offset and hex offset
- access width and signedness
- read/write direction
- functions and replay scenarios where observed
- candidate field name/type
- static and runtime evidence

Cluster offsets by common base and lifetime. Define structs only after the cluster is stable. Use types to improve analysis, then re-check the disassembly for circular reasoning.

Search address construction pairs (`lui` plus `addiu`/`ori`), pointer tables, GP-relative accesses, BIOS vectors, DMA/GPU command buffers, callbacks, and overlay descriptors. Distinguish literal values from relocated pointers.

### 8. Ingest symbols and signatures

Search exhaustively for:

- shipped or leaked-with-authorization `.SYM`, `.MAP`, `.CPE`, ELF, `OBJ`, `LIB`
- strings containing source paths, assertions, version identifiers, or function names
- PsyQ library signatures
- sibling regional/revision binaries with more symbols or less optimization
- demo/prototype builds and debug executables

Confidence order:

```text
exact original symbol > exact symbolized-library signature > exact cross-version match
> constrained structural/signature match > heuristic semantic name
```

Use `scripts/symbols_to_rizin.py` to convert reviewed CSV/JSON symbols into a Rizin command script. Apply generated scripts to a copy/project first and retain the input provenance.

Use Rizin FLIRT support and `rz-sign` where suitable. Use the Ghidra PSX loader and PsyQ signature database as an independent corroborating tool, not as unquestioned truth.

### 9. Map overlays independently

An overlay is not merely another section. Determine:

- source file/archive entry and hash
- compression/decompression algorithm
- destination address and size
- loader function and call sites
- relocation/fixup behavior
- entrypoints, callbacks, jump tables, and lifetime
- which replay/scenario loads it
- which previous overlay it replaces

Create one namespace/project per overlay identity. Never merge functions solely because two overlays occupy the same runtime range.

Use runtime write breakpoints on the destination range and execution breakpoints at candidate entrypoints. Capture a memory dump after relocation/decompression and compare it with the on-disc form.

### 10. Use every replay and scripted path as evidence

“Use all replays” means build and maintain a replay corpus, not casually run one demo.

Inventory:

- built-in attract/demo sequences
- replay/ghost files
- memory-card saves
- scripted tutorials/cutscenes
- debug menus and level selects
- emulator input movies created for reproducible scenarios

Use BizHawk or another deterministic movie-capable emulator to record/replay input scenarios. Use PCSX-Redux for execution/read/write breakpoints, register/memory capture, Lua instrumentation, and GDB access. DuckStation CPU logs may be used as a high-volume cross-check, but constrain them because traces can be very large.

Each replay matrix row must include hashes, emulator/core, BIOS, region, starting state, frame range, expected state, overlays loaded, functions hit, memory watched, and result.

A function is not runtime-covered until at least one reproducible replay hits it with recorded evidence. A feature is not exhaustively covered until every known replay/path is explicitly marked passed, failed, blocked, duplicate, or not applicable.

### 11. Reconcile static and dynamic evidence

After each trace pass:

- import confirmed function starts and indirect targets
- add reviewed xrefs
- update argument/value observations
- rename only at the earned confidence level
- annotate overlay identity and lifetime
- rerun focused analysis, not an indiscriminate global reset
- export artifacts again and diff them

When evidence conflicts, preserve both observations and state the likely cause: wrong load base, alias mismatch, stale overlay, dynarec/debug settings, corrupted function boundary, GP mismatch, delay-slot oversight, or signature collision.

### 12. Decompile only after control flow is credible

Use:

```text
pdf     canonical function disassembly
pdg     rz-ghidra decompilation hypothesis
pdgo    decompilation with offsets
pdgj    machine-readable decompiler output
```

Decompiler output is a drafting aid. Verify every loop, switch, signed comparison, pointer arithmetic expression, return type, and side effect against instructions and runtime observations.

### 13. Matching decompilation when requested

Use the established PSX community toolchain where appropriate:

- splat for binary splitting/configuration
- spimdisasm for MIPS disassembly and symbol-aware splitting
- PsyQ/GCC-compatible compiler and assembler setup
- maspsx for PsyQ assembler compatibility
- asm-differ and/or objdiff for function/object comparison
- decomp-permuter and decomp.me for controlled matching exploration

Keep semantic recovery and byte matching as separate milestones. A matching function can still be wrongly named or typed; a nonmatching function can be semantically correct.

Use `bin/build-diff` as a repository-configurable wrapper. Never invent a build command when the project already has one.

### 14. Deliver evidence, not just conclusions

Every final report must contain:

- exact input hashes and tool versions
- binary/overlay inventory
- address-map formulas and proven load bases
- function table with confidence and coverage
- argument/type/offset findings with caller and trace evidence
- unresolved indirect calls/jump tables
- replay coverage and trace references
- symbol/signature provenance
- matching status if applicable
- contradictions, open questions, and next highest-value experiment

Mark unsupported conclusions as `[INFERRED]` and state the evidence chain.

## Completion gates

Do not declare the case complete until these gates are reviewed:

- [ ] All input containers and binaries inventoried and hashed
- [ ] Boot executable and every discovered overlay mapped
- [ ] File/runtime/overlay coordinates recorded consistently
- [ ] Known symbol and signature sources searched
- [ ] Function boundaries and indirect dispatch reviewed
- [ ] Arguments and repeated object offsets ledgered
- [ ] All known built-in and recorded replays have a status
- [ ] Runtime evidence merged back into static analysis
- [ ] Decompiler output checked against MIPS instructions
- [ ] Build/diff evidence captured when matching is in scope
- [ ] Proprietary inputs excluded from distributable artifacts

## Bundled utilities

```text
scripts/psx_exe.py              inspect/extract PS-X EXE and convert addresses
scripts/scan_mips.py            triage raw modules for RAM pointers and call/jump candidates
scripts/rizin_export.py         export inventory through Rizin JSON commands
scripts/function_artifacts.py   dump disassembly, decompilation, xrefs, variables, and JSON
scripts/symbols_to_rizin.py     convert reviewed symbols to Rizin commands
scripts/replay_coverage.py      validate and summarize replay-matrix coverage
bin/lift                        inspect/extract/map a PS-X EXE or raw overlay in Rizin
bin/build-diff                  run configured build and compare outputs
bin/psx-rizin                   convenience dispatcher for bundled utilities
```
