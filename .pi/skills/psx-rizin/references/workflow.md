# End-to-end PS1 reverse-engineering workflow

## Phase 0 — Define the question

[owner](../SKILL.md#route-the-task): record a testable target, scope, deliverables, and proof. Examples: identify PsyQ calls; recover a function/callers; explain a system/structures; map overlay loading; produce matching C; compare revisions. Replace “understand engine” with bounded hypotheses/replays.

## Phase 1 — Provenance / immutable inputs

[owner](runtime-and-replays.md):
1. Case ID.
2. Hash disc tracks, executables, overlays, BIOS, symbols, replays, sibling revisions.
3. Record extraction tools/commands.
4. Keep originals read-only.
5. Use `out/`: snapshots `out/reverse/snapshots/<encoded-target>.json` (`bin/harness analysis rz-project`), index `out/index/`, matching `out/matching/`, `out/permuter/`, `out/asm-diff/`; else use `.agent-work/psx-rizin/<case-id>/`.

Manifest fields: `templates/case-manifest.yaml`.

## Phase 2 — Disc / filesystem inventory

[owner](overlays-and-assets.md): parse `SYSTEM.CNF` to boot path; inventory every ISO entry and raw track range, including `.BIN`, `.DAT`, extensionless files. Per candidate record magic/entropy, PS-X EXE magic, MIPS calls/jumps, RAM-looking words, strings/paths/assertions, alignment/padding, loader read sizes/CD sectors, and raw/compressed/encrypted/relocated/interpreted state. Raw MIPS scanning is triage, not proof.

## Phase 3 — Address model

[owner](psx-abi-and-addressing.md): make this table before naming.

| identity | source offset | runtime range | alias range | lifetime | evidence |
|---|---:|---:|---:|---:|---|
| main payload | file + `0x800` | header text address | KSEG aliases | process | PS-X EXE header |
| overlay A | raw + `0` | loader destination | KSEG aliases | level 1 | write breakpoint |

Classify addresses: file offset · sector/LBA + intra-sector · executable payload offset · runtime VA · physical RAM · overlay-relative. Use a PS-X EXE parser for header conversions; never apply the 0x800 formula to an arbitrary overlay.

## Phase 4 — Static baseline

[owner](rizin-playbook.md): extract EXE payload, map at text load address, analyze conservatively; export baseline JSON before manual edits. Review startup, GP/stack init, BIOS calls/vectors, `.ctors`, library startup/heap, main loop, callback registration, CD/file loader, overlay manager, controller input, VBlank, GPU ordering tables, and sound/CD-XA paths.

## Phase 5 — Function boundaries

[owner](rizin-playbook.md): roots in order: entry → direct `jal` targets → callback pointers → jump-table targets → runtime-executed addresses → symbol/signature matches → plausible prologues (lowest). Validate incoming/outgoing edges, delay slots, tail calls, shared epilogues, literal/data pools, merged neighbors, and non-returning functions. Store commands, disassembly, xrefs, decompiler text, trace observations as function-local artifacts—not screenshots.

## Phase 6 — Calls / args / returns / globals

Per call ([owner](psx-abi-and-addressing.md)): a0–a3 definitions → stack arg stores → `jal`/`jalr` delay slot → target’s first arg reads → v0/v1 consumers → all call sites → runtime values across representative replays. Separate facts (“a0 read at offset 0x18”) from inference (`[INFERRED] a0 is a Player pointer`). GP may vary by module/function; never force one value across overlays.

## Phase 7 — Structures / offset ledger

[owner](symbols-signatures-and-types.md): collect accesses before types, keyed by `(base-role, allocation/lifetime, offset, width, signedness)`. Correlate static offsets with watchpoints/replay transitions; names must explain all known reads/writes. Distinguish arrays from structures; derive stride from MIPS multiply/shift and verify `index * stride + field`.

## Phase 8 — Indirect control flow / xrefs

Per `jalr`/register jump ([owner](command-reference.md#xrefs)): backward-slice target → table base/index → element width → absolute/relative/relocated → bounds/default → enumerate targets → breakpoint/replay validation → reviewed manual xrefs. Cover state dispatch, vtables, callback arrays, BIOS/library callbacks, overlay entry tables, switches, and script opcodes. A RAM-looking word never proves an xref.

## Phase 9 — Symbols / library ID

[owner](symbols-signatures-and-types.md): search symbol-bearing sources before renaming; import exact symbols → signatures → cross-version matches → heuristics, with provenance per name. PsyQ ID requires exact signatures plus call graph, constants, MMIO/BIOS use, strings/assertions, parameter behavior, and sibling-build matches. Mark colliding short wrappers ambiguous.

## Phase 10 — Overlay lifecycle

[owner](overlays-and-assets.md): prove source on disc, read/decompression size, destination, cache flush, relocation/fixups, entrypoint registration, and unload/replace event. Build replay timelines; compare raw files to runtime dumps to expose decompression, relocation, or mutable data appended to code.

## Phase 11 — Replay-driven dynamic analysis

[owner](runtime-and-replays.md): use minimal deterministic built-in and recorded scenarios. Instrument function entry/return, indirect targets, overlay writes, alloc/init, field reads/writes, file/CD reads, input, frame/VBlank, DMA/GPU/SPU. Log frame, PC, caller/RA, a0–a3, stack words, return v0/v1, and memory snapshots. No unlimited full CPU traces before narrowing the window.

## Phase 12 — Instruction reconciliation

[owner](command-reference.md#core): reconcile Rizin `pdf`/`pd` instructions, xrefs,
proposed C and runtime facts; preserve disagreements. Check delay slots, signedness,
pointer aliasing, GP-relative globals, switches, shared tails/epilogues, 64-bit
register pairs and GTE macro semantics.

## Phase 13 — Matching decompilation

Start only after semantic recovery stabilizes. Pin compiler/assembler/linker versions, section order, symbol map, and split; match one function at a time and record flags/score. Asm diffs evidence compiler behavior, not understanding. Where feasible, rerun runtime scenarios after replacement. See [decomp-build-diff.md](decomp-build-diff.md).

## Phase 14 — Audit / handoff

[owner](../SKILL.md#deliver): deliver `inventory.md`, `address-map.md`, `functions.csv`, `offset-ledger.csv`, `symbols.csv`, `overlays.md`, `replay-coverage.csv`, `open-questions.md`, per-function artifact directories, and reproducible commands/scripts. Every conclusion must be reproducible from hashes, commands, and evidence—not private memory.
