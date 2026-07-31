---
type: Runtime research
title: Historical GCC MIPS scheduling residuals
description: Evidence and bounded next steps for register-allocation and delay-slot mismatches.
tags: [compiler, matching, mips, evidence]
---

# Historical GCC MIPS scheduling residuals

This is a research guide for clean-C matching residuals under the repository's
`gcc-2.7.2-psx` / maspsx / ASPSX chain. It does not prove that retail BOF3 used
a particular compiler flag or permit a profile override. Only a live exact
`bin/byte-match` does either.

## What the compiler passes can change

The GNU GCC 2.95 optimisation manual is the closest authoritative public manual
to the bundled GCC 2.7.2 vintage. It states that:

- `-O` enables `-fdelayed-branch` on machines with delay slots;
- `-O2` enables almost all optional optimizations;
- `-fregmove` (also called `-foptimize-register-moves`) tries to reassign
  registers in moves and simple operands to improve register tying;
- `-fdelayed-branch` tries to fill delayed branch slots; and
- `-fschedule-insns` / `-fschedule-insns2` are instruction scheduling passes.

The repository's actual `toolchains/gcc-2.7.2-psx/cc1` recognises the related
option strings `thread-jumps`, `force-mem`, `delayed-branch`, `schedule-insns`,
and `schedule-insns2`, as well as MIPS delay-slot scheduling routines. This is
**capability evidence**, not evidence that any flag reproduces an original.

## What source shape can control

GCC assigns registers from value live ranges, not from source parameter names.
For a function whose original begins `move t0,a1`, the clean source must make
that `a1` value live early and across the point where `a1` is reused; it must
not preserve another parameter merely because that gives a superficially closer
first instruction. The next clean-C levers are:

1. represent the actual input roles and widths, including pointer versus value;
2. load fields in their original use order, so an input pointer can die/reuse at
   the same point;
3. make the copied input's lifetime explicit with one readable local and use it
   consistently in both comparisons;
4. choose the observed result CFG (`result = 0; if (...) result = ...; return
   result;` versus early return) before considering a compiler profile; and
5. only then search a bounded set of statement/declaration order variants.

A source-shape improvement is not acceptance: preserve only an exact match.

## Permuter limits

The upstream decomp-permuter README pinned in this repository says it is
strongest late, when remaining differences are mostly register allocation. It
is poor at stack differences, while reordered or functional differences are
usually better resolved by hand. It can discover promising changes accidentally,
so translate any winner back into readable C and independently byte-match it.

## Profile experiment gate

A profile experiment is appropriate only after the source has the correct size,
control flow, accesses, and first-diff category is scheduling/allocation. Test
one candidate at a time from the compiler's supported switches, retaining an
object override in `config/compiler/object-flags.cmake` only when it produces a
clean-C live exact match.

`bin/flag-search` currently requires exactly one source row in
`compile_commands.json`; a newly-created lift has none until it is registered
in the build. Do not fake a tracked source or change repository configuration
solely to satisfy that lookup. Use the equivalent disposable compile command
(the permuter workspace records one) for research, or improve the tool with a
tested explicit-source fallback before relying on broad automated flag search.

## `battle/15@0x800AF66C` application

The parameter roles are documented in
[`battle-range-predicates.md`](battle-range-predicates.md): `a0` is the range
pointer and `a1` is the extent copied into `t0`. The unresolved entry move is
therefore an allocator/scheduling residual after the source model is correct;
it does **not** justify reversing the parameters or adding a global macro.

A bounded profile experiment was run on 2026-07-30 against the strongest
same-size clean-C permuter candidate, using its disposable compile command.
None matched the entry register web: baseline and all accepted flag spellings
still began `move a3,a1`, rather than original `move t0,a1; move v0,zero`.

| Candidate | Size | Result |
| --- | ---: | --- |
| baseline | 76 | 19 instructions, but zero position matches; entry used `a3` for value |
| `-fno-delayed-branch` | 84 | Added two instructions; reject |
| `-fno-schedule-insns` | 76 | Changed instruction order but still used `a3` for value |
| `-fno-schedule-insns2` | 76 | Same failure; entry used `a3` |
| both scheduling flags | 96 | Added five instructions; reject |
| `-fno-thread-jumps` | 76 | No useful change |
| `-fno-force-mem` | 76 | Changed other registers but still began `move a3,a1` |
| `-fno-regmove` | — | Unsupported by bundled `cc1`; reject the flag rather than infer behavior |

The next clean-C step is not a wider flag search. Recover a source shape whose
baseline assigns the extent to `t0`; only then may a single compatible profile
flag be retested. Reject every result that changes function size, control flow,
or fails live byte matching.

## Sources

- [GCC 2.95.3 optimization options](https://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_2.html), §§2.8 and 2.14.14 (accessed 2026-07-30).
- [GCC internals: delayed branch scheduling](https://gcc.gnu.org/onlinedocs/gccint/RTL-passes.html) (conceptual description; current documentation).
- `third_party/decomp-permuter/README.md` at pinned commit `efc5c5e`.
- `toolchains/gcc-2.7.2-psx/cc1` option-string inspection; repository-local capability check.
