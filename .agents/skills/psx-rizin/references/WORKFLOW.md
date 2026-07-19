# End-to-end PS1 reverse-engineering workflow

## Phase 0 — Define the question

Start with a testable target, for example:

- identify every PsyQ call in all executables and overlays
- recover a function at a runtime address and all callers
- explain a gameplay system and its data structures
- map an overlay manager and every load destination
- produce matching C for a function or module
- compare revisions or regional builds

Record scope, expected deliverables, and what constitutes proof. A task such as “understand the engine” must be decomposed into bounded hypotheses and replay scenarios.

## Phase 1 — Provenance and immutable inputs

1. Create a case ID.
2. Hash disc tracks, executable files, overlays, BIOS, symbols, replay files, and any sibling revision.
3. Record extraction tools and commands.
4. Keep original inputs read-only.
5. In the BOF3 repository, store generated files under the repo's disposable
   `out/` tree: Rizin snapshots in `out/reverse/<target>/` (via `bin/rz-project`),
   the query cache in `out/index/`, and matching workspaces in `out/matching/`,
   `out/permuter/`, and `out/asm-diff/`. Outside this repo, use
   `.agent-work/psx-rizin/<case-id>/` if present.

Recommended manifest fields are in `templates/case-manifest.yaml`.

## Phase 2 — Disc and filesystem inventory

Locate and parse `SYSTEM.CNF` to identify the boot path. Inventory all ISO entries and raw track ranges. Do not ignore files with generic extensions such as `.BIN`, `.DAT`, or extensionless names.

For every candidate executable/module:

- inspect magic and entropy
- search for PS-X EXE magic
- search for MIPS call/jump encodings and RAM-looking words
- search for strings/source paths/assertions
- identify alignment/padding patterns
- compare against loader read sizes and CD sector requests
- record whether data is raw, compressed, encrypted, relocated, or interpreted

Use `scripts/scan_mips.py` only as triage. Its candidates are not proof of executable code.

## Phase 3 — Establish the address model

Create an address-space table before naming functions:

| identity | source offset | runtime range | alias range | lifetime | evidence |
|---|---:|---:|---:|---|---|
| main payload | file + `0x800` | header text address | KSEG aliases | process | PS-X EXE header |
| overlay A | raw + `0` | loader destination | KSEG aliases | level 1 | write breakpoint |

For each address seen in notes, state one of:

- file offset
- disc sector/LBA and intra-sector offset
- executable payload offset
- runtime virtual address
- physical RAM offset
- overlay-relative offset

Use `scripts/psx_exe.py` for PS-X EXE conversions. Never apply the 0x800-header formula to an arbitrary overlay.

## Phase 4 — Static baseline

Extract the PS-X EXE payload and map it at the text load address. Run conservative analysis. Then review:

- entry point and startup sequence
- GP initialization
- stack initialization
- BIOS calls and vectors
- `.ctors`-like initialization patterns
- library startup and heap setup
- main loop candidates
- callback registration
- CD/file loader
- overlay manager
- controller/input read path
- frame/VBlank synchronization
- GPU ordering-table construction
- sound/CD-XA paths

Export baseline JSON before extensive manual edits. This gives a diffable starting point.

## Phase 5 — Function-boundary recovery

Prioritize roots with strong evidence:

1. executable entry point
2. direct `jal` targets
3. callback pointers written to known APIs/data
4. jump-table targets
5. runtime-executed addresses
6. symbol/signature matches
7. plausible prologues only as lower-confidence candidates

For each function, validate:

- all incoming control-flow edges
- all outgoing direct/indirect edges
- delay slots
- tail calls
- shared epilogues
- literal/data pools mistakenly included
- adjacent functions merged by analysis
- non-returning functions

Use function-local artifacts rather than screenshots. Store commands, disassembly, xrefs, decompiler text, and trace observations.

## Phase 6 — Calls, arguments, returns, and globals

At each call:

1. locate definitions of `a0`–`a3`
2. inspect stack argument stores
3. inspect the `jal`/`jalr` delay slot
4. inspect the target’s first reads of arguments
5. inspect return-value consumers (`v0`/`v1`)
6. compare all call sites
7. capture runtime values across representative replays

Separate facts:

- “a0 is read at offset 0x18”
- “a0 points to a mutable object”
- “[INFERRED] a0 is a Player pointer”

Do the same for globals and GP-relative data. GP can vary by module or function; do not force one value over all overlays.

## Phase 7 — Structures and offset ledger

Accumulate accesses before defining a type. A useful grouping key is:

```text
(base-role, allocation/lifetime, offset, width, signedness)
```

Correlate static offsets with runtime watchpoints and replay state transitions. A field name should explain reads and writes across all known users, not just one function.

Record arrays separately from structures. MIPS multiplication/shift patterns often expose element stride. Check whether an apparent field offset is actually `index * stride + field`.

## Phase 8 — Indirect control flow and xrefs

For every `jalr` or register jump:

- backward-slice the target register
- identify table base and index
- determine element width and whether values are absolute, relative, or relocated
- determine bounds/default path
- enumerate targets
- validate targets with execution breakpoints or replay coverage
- add reviewed manual xrefs

Common patterns include:

- state-machine dispatch
- virtual/object method tables
- callback arrays
- BIOS/lib callbacks
- overlay entrypoint tables
- switch jump tables
- script opcode dispatch

Do not add xrefs merely because a word falls in RAM. Require contextual evidence.

## Phase 9 — Symbols and library identification

Search all symbol-bearing sources before semantic renaming. Import exact symbols first, then signatures, then cross-version matches, then heuristics. Keep a provenance field for every name.

For PsyQ identification, combine:

- exact library/OBJ signatures
- call graph shape
- constants and MMIO/BIOS usage
- strings/assertions
- parameter behavior
- sibling-build matches

Signature collisions are possible, especially for short wrapper functions. Mark ambiguous hits.

## Phase 10 — Overlay lifecycle

Find the loader and prove:

- source on disc
- read/decompression size
- destination
- cache flush behavior if present
- relocation/fixups
- entrypoint registration
- unload/replacement event

Create an overlay timeline per replay. Static analysis of the raw file and runtime dump should be compared byte-for-byte; differences often reveal decompression, relocation, or mutable data appended to code.

## Phase 11 — Replay-driven dynamic analysis

Create minimal deterministic scenarios that isolate behavior. Use all discovered built-in replays plus recorded scenarios.

Instrumentation targets:

- function entry/return
- indirect call target
- overlay destination writes
- object allocation/initialization
- field read/write
- file/CD reads
- input state
- frame counter/VBlank
- DMA/GPU/SPU registers where relevant

Log frame number, PC, caller/RA, `a0`–`a3`, stack words, `v0`/`v1` on return, and selected memory snapshots. Avoid unlimited full CPU traces until a narrow frame/function window is known.

## Phase 12 — Decompiler reconciliation

Compare `pdf`, `pdgo`, and runtime facts. Rewrite misleading constructs manually when necessary. Typical MIPS decompiler hazards:

- delay-slot assignment moved across calls/branches
- signed versus unsigned comparisons
- pointer aliasing
- GP-relative globals
- switch reconstruction
- shared tail/epilogue blocks
- 64-bit arithmetic formed from register pairs
- GTE macro semantics

The decompiler is allowed to be wrong. The report must not hide disagreement.

## Phase 13 — Matching decompilation

Only begin byte matching after semantic recovery is stable enough. Establish compiler/assembler/linker versions, section ordering, symbol map, and binary split. Match one function at a time and record compiler flags and score.

Use assembly diffs as evidence of compiler behavior, not as a substitute for understanding. Re-run runtime scenarios after replacing a function in a mod/test build where feasible.

## Phase 14 — Audit and handoff

Produce:

- `inventory.md`
- `address-map.md`
- `functions.csv`
- `offset-ledger.csv`
- `symbols.csv`
- `overlays.md`
- `replay-coverage.csv`
- `open-questions.md`
- per-function artifact directories
- reproducible commands/scripts

A new analyst should be able to regenerate each conclusion from hashes, commands, and evidence without relying on private memory.
