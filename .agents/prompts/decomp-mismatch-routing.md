---
description: Route BOF3 PSX mismatch classes to the right next step
agent: build
subtask: true
---

## Purpose

Use this note to decide whether a remaining mismatch is worth chasing in pure C,
worth testing with `maspsx` / ASPSX-version behavior, or likely a dead end for
the current workflow.

All routing in this note is subordinate to the same-object/asm requirement: the
goal is not merely nicer C, but nicer C that still produces the same verified
canonical object/asm.

This note is a companion to:

- `@.opencode/commands/decomp.md`
- `@.opencode/commands/decomp-worker.md`
- `@.opencode/commands/decomp-checkpoint.md`

## Readiness Gate

Before calling something a mismatch class, check whether the workspace is even
ready for backend diffing.

Prefer:

- `python3 -m scripts.rebof3 match diff --workspace-json <path>`
- or `make match_diff PROGRAM=... ENTRY=...`

Route these statuses before deeper mismatch analysis:

- `blocked_missing_ghidra_bundle`: rerun the recorded `ghidra_decomp` command or
  use `--refresh-ghidra-bundle`
- `needs_build_status` or `blocked_build_failed`: run `make match_build` and fix
  the PSX build first
- `blocked_missing_source_mapping`: go back to import / promote / seed / stub /
  lift work instead of blaming `maspsx`
- `blocked_missing_expected_baseline`: refresh the workspace after the Ghidra
  bundle exists so the expected baseline can be recorded

Only do mismatch routing once the workspace is effectively
`ready_for_backend_diff`.

## First Question

Ask this first:

- what does `match diff` say the workspace is blocked on, if anything?
- what do `func.s`, `func.m2c.c`, and Ghidra inspection suggest about the
  current mismatch class?
- did headless `function callers` / `function refs` already prove the real
  entry point and any fixed data addresses involved in the mismatch?
- if `func.m2c.c` is missing or failed, is one manual rerun worth trying first?
- is the mismatch already present in GCC output before `maspsx`?
- if the class is still unclear from stdout metrics, what does `make match_view`
  show in the side-by-side rows?

If yes:

- default to a source-shape or compiler-shape diagnosis
- stop blaming `maspsx` unless the remaining mismatch class is known to be
  `maspsx`-driven

If no:

- check whether the transformed assembly matches a known `maspsx` / ASPSX
  version bucket

Manual `m2c` retry, when useful:

- `python3 third_party/tools/m2c/m2c.py -t mipsel-gcc-c func.s`
- if it still fails, continue from asm + Ghidra inspection without blocking

## When To Try `--aspsx-version`

Try an ASPSX-version sweep only when the last mismatch clearly looks like one
of these classes:

- `$at` expansion behavior
- inserted or missing `nop` around `$at` expansion
- `addiu` vs `addu` inside `$at` expansion
- `%hi/%lo` macro-related `nop` behavior
- `li 1` expansion behavior
- `$gp` / non-zero `-G` behavior
- div/mult hazard handling:
  - `mflo`
  - `mfhi`
  - `mthi`
  - `mtlo`
  - branch-to-`nop` hazard differences

Do not sweep versions just because a function is close.

## When To Stop Blaming `maspsx`

Stop blaming `maspsx` when:

- GCC already emits the wrong operation ordering before `maspsx`
- the mismatch is a generic register-allocation or source-ordering issue
- the mismatch is a plain pseudo-op choice unrelated to known `maspsx`
  transforms
- all relevant ASPSX versions produce the same transformed lines
- upstream `maspsx` issues and PRs explain the current output as intentional
  behavior

For the current repo workflow, this includes:

- commutative `addu` operand-order gaps that originate from large numeric
  absolute operands
- generic `move` / `or` / `addu` leaf encoding differences
- plain GCC `ori` vs `addiu` address-materialization choices before any
  `$at`-rewrite happens

## Known Useful `maspsx` Levers

Relevant levers:

- `--aspsx-version`
- `-G`
- `--expand-div`
- `--use-comm-section`

Usually not relevant unless the mismatch class proves it:

- `--dont-expand-li`
- plumbing/debug flags

## `$at` Routing Rule

Distinguish these two cases:

1. symbol/addend path
2. large numeric operand path

If GCC emits a symbol/addend form such as:

- `lw $v0,symbol($a0)`

then `maspsx` takes the symbol/addend path and typically expands through:

- `lui $at,%hi(symbol)`
- `addu $at,$at,$a0`
- `lw $v0,%lo(symbol)($at)`

If GCC emits a large numeric operand such as:

- `lhu $2,49344($2)`

then `maspsx` takes the large-numeric path and expands through:

- `lui $at,%hi(49344)`
- `addu $at,$2,$at`
- `lhu $2,%lo(49344)($at)`

Implication:

- if the unwanted operand order comes from the large-numeric path, the direct
  cause is `maspsx`
- but pure C can still help indirectly if it changes GCC output so the access
  becomes a symbol/addend-style form or otherwise avoids the large-numeric path

Do not expect a `maspsx` flag to flip this ordering for the canonical flow.
Upstream history indicates this behavior is intentional for negative / large
numeric operands.

## Likely Pure-C Wins

Keep pushing in pure C when the mismatch looks like:

- signed vs unsigned temp width
- cast placement
- helper inlining forcing bad address folding
- guessed raw addresses that can be replaced by xref-proven local
  defines/externs/tables
- raw absolute base vs `0x80140000`-style high-base spelling
- typed pointer vs byte-pointer address formation
- temp introduction or removal
- early-return or condition reshaping
- loop-shape or induction-variable differences

If a pure-C cleanup looks nicer but cannot be made to produce the same
object/asm, route it as cleanup-for-later, not as the active matching answer.

Use the fast local `make match_view` loop to classify the next instruction-level
move, then use the durable backend diff before declaring the result good enough
to keep.

## Likely Pure-C Dead Ends

Deprioritize unless a new lever appears:

- tiny leafs blocked only by GNU `as` pseudo-op encoding:
  - `move` becoming `addu`
  - expected `or` but emitted `addu`
- relocation-name-only diffs
- branch-delay / `nop` differences that map exactly to a known `maspsx` bug
  already fixed upstream and already present in the vendored copy
- operand-order differences that remain identical across all relevant ASPSX
  versions and source-shape attempts

## Worker Decision Rule

For a one-file worker:

1. inspect `match diff` first so you know whether the workspace is actually
   ready for backend diffing
2. inspect GCC output before `maspsx`
3. inspect `make match_view` if the remaining gap is easier to read side by side
   than from percent/mismatch counts alone
4. compare asm, Ghidra C, and `m2c` output when available before calling the
   mismatch a ceiling
5. classify the mismatch
6. choose one of:
    - pure-C source-shape iteration
    - targeted ASPSX-version sweep
    - standard 60-second permuter pass after a manual plateau
    - stop as likely ceiling
    - route back to import / promote / seed / repair / lift work if the issue is
      not actually a backend-ready mismatch yet
7. report which class the function fell into

## Short Output Format

When reporting a routed mismatch, use:

- workspace readiness status
- mismatch class
- shows up before or after `maspsx`
- whether version sweep is worth trying
- whether pure C is still promising
- whether the function looks like a likely ceiling
