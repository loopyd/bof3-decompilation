# End-to-end PS1 reverse-engineering workflow

## Phase 0 — Define the question

Testable target, e.g.: identify PsyQ calls everywhere; recover a function + all callers; explain a system + data structures; map overlay manager + load destinations; produce matching C; compare revisions/regional builds. Record scope, deliverables, proof. "Understand engine" → decompose into bounded hypotheses + replay scenarios.

## Phase 1 — Provenance / immutable inputs

1. Case ID.
2. Hash disc tracks, executables, overlays, BIOS, symbols, replays, sibling revisions.
3. Record extraction tools/commands.
4. Keep originals read-only.
5. In this repo, generated files live in `out/`: snapshots `out/reverse/snapshots/<encoded-target>.json` (`bin/rz-project`), index `out/index/`, matching `out/matching/`, `out/permuter/`, `out/asm-diff/`. Outside: `.agent-work/psx-rizin/<case-id>/`.

Manifest fields: `templates/case-manifest.yaml`.

## Phase 2 — Disc / filesystem inventory

Parse `SYSTEM.CNF` → boot path. Inventory all ISO entries + raw track ranges; don't skip `.BIN`/`.DAT`/extensionless. Per candidate: magic/entropy, PS-X EXE magic, MIPS call/jump encodings, RAM-looking words, strings/source paths/assertions, alignment/padding, loader read sizes + CD sector requests, raw/compressed/encrypted/relocated/interpreted. Raw MIPS scanning = triage only, not proof of code.

## Phase 3 — Address model

Address-space table before naming:

| identity | source offset | runtime range | alias range | lifetime | evidence |
|---|---:|---:|---:|---:|---|
| main payload | file + `0x800` | header text address | KSEG aliases | process | PS-X EXE header |
| overlay A | raw + `0` | loader destination | KSEG aliases | level 1 | write breakpoint |

State each noted address as one of: file offset · sector/LBA + intra-sector · executable payload offset · runtime VA · physical RAM · overlay-relative. Use a PS-X EXE parser for header conversions. Never apply the 0x800 formula to an arbitrary overlay.

## Phase 4 — Static baseline

Extract EXE payload, map at text load address, conservative analysis. Review: entry/startup, GP init, stack init, BIOS calls/vectors, `.ctors`, library startup/heap, main loop, callback registration, CD/file loader, overlay manager, controller input, VBlank, GPU ordering tables, sound/CD-XA paths. Export baseline JSON before manual edits.

## Phase 5 — Function boundaries

Root priority: entry point → direct `jal` targets → callback pointers → jump-table targets → runtime-executed addresses → symbol/signature matches → plausible prologues (lowest). Validate: incoming edges, outgoing direct/indirect edges, delay slots, tail calls, shared epilogues, literal/data pools, merged adjacent functions, non-returning functions. Use function-local artifacts, not screenshots. Store commands, disassembly, xrefs, decompiler text, trace observations.

## Phase 6 — Calls / args / returns / globals

Per call: define a0–a3 → stack arg stores → jal/jalr delay slot → target's first arg reads → return-value consumers (v0/v1) → all call sites → runtime values across representative replays. Separate facts: "a0 read at offset 0x18" vs "[INFERRED] a0 is a Player pointer". Globals: GP can vary per module/function; don't force one value over all overlays.

## Phase 7 — Structures / offset ledger

Accumulate accesses before defining a type. Grouping key: `(base-role, allocation/lifetime, offset, width, signedness)`. Correlate static offsets with runtime watchpoints + replay transitions. A field name must explain reads/writes across ALL known users. Arrays ≠ structures; MIPS mul/shift patterns expose element stride; check `index * stride + field`.

## Phase 8 — Indirect control flow / xrefs

Per `jalr`/register jump: backward-slice target reg → table base/index → element width → absolute/relative/relocated → bounds/default → enumerate targets → validate via breakpoints/replay → add reviewed manual xrefs. Patterns: state dispatch, vtables, callback arrays, BIOS/lib callbacks, overlay entry tables, switch tables, script opcode dispatch. Require contextual evidence; never xref just because a word falls in RAM.

## Phase 9 — Symbols / library ID

Search all symbol-bearing sources before semantic renaming. Import order: exact symbols → signatures → cross-version matches → heuristics. Provenance per name. PsyQ ID: exact signatures + call graph shape + constants + MMIO/BIOS usage + strings/assertions + parameter behavior + sibling-build matches. Short wrappers collide — mark ambiguous hits.

## Phase 10 — Overlay lifecycle

Find the loader and prove: source on disc, read/decompression size, destination, cache flush, relocation/fixups, entrypoint registration, unload/replace event. Overlay timeline per replay. Compare raw file vs runtime dump byte-for-byte; differences reveal decompression, relocation, or mutable data appended to code.

## Phase 11 — Replay-driven dynamic analysis

Minimal deterministic scenarios from built-in replays + recorded scenarios. Instrument: function entry/return, indirect call targets, overlay destination writes, alloc/init, field r/w, file/CD reads, input state, frame/VBlank, DMA/GPU/SPU. Log frame, PC, caller/RA, a0–a3, stack words, v0/v1 on return, memory snapshots. No unlimited full CPU traces until a narrow window is known.

## Phase 12 — Decompiler reconciliation

Compare `pdf`/`pdgo` + runtime facts; rewrite misleading constructs. Hazards: delay-slot assignment moved across calls/branches, signed vs unsigned comparisons, pointer aliasing, GP-relative globals, switch reconstruction, shared tail/epilogue blocks, 64-bit register pairs, GTE macro semantics. Decompiler may be wrong; report must not hide disagreement.

## Phase 13 — Matching decompilation

Begin only after semantic recovery is stable. Pin compiler/assembler/linker versions, section ordering, symbol map, binary split. Match one function at a time; record flags + score. Asm diffs are evidence of compiler behavior, not a substitute for understanding. Re-run runtime scenarios after replacing a function where feasible. See DECOMP_BUILD_DIFF.md.

## Phase 14 — Audit / handoff

Deliver: `inventory.md`, `address-map.md`, `functions.csv`, `offset-ledger.csv`, `symbols.csv`, `overlays.md`, `replay-coverage.csv`, `open-questions.md`, per-function artifact directories, reproducible commands/scripts. A new analyst must regenerate every conclusion from hashes/commands/evidence — no private memory.
