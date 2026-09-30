# Case 053 division guards

## Input identity

The inspected function is `emi/battle/battle/03@0x801DB058`, with a 672-byte
extent ending at `0x801DB2F8`. Its original-byte SHA-256 is
`6f2cc0eb4f2eb121ac54f6aea7ea7e0546c69c76b0bcf0b01a48511dde0a262b`.
The containing binary SHA-256 is
`f25ef137618bc024a6aec98c2c4d9a571c6f23884762fb4567406e6552b383c1`.

A diagnostic driver linked against the rebuilt plugin captured decoder output
before ABI recovery or LLVM optimization. The retained files are
`out/retdec-psx/guard-analysis/dump_0.ll`, `input.json`,
`original-disassembly.txt`, and `eligibility-disassembly.txt`.
The original instructions contain two signed DIV sequences, each with a
zero-divisor BREAK and a signed-overflow BREAK. Later unsigned source recovery
does not change the original instruction's identity.

## First loop and division

At entry, `s3`, the sum, and `s2`, the count, are zero. `s1` starts at 3.
The loop executes for indices 3 through 10, eight iterations. At each iteration:

- The eligibility call at `0x801DB090` either skips both updates or reaches both.
- A selected unsigned halfword is loaded at `0x801DB0C0`.
- `s3 += value` executes in the branch delay slot at `0x801DB0D0`, regardless of
  whether the maximum is updated.
- `s2 += 1` executes at `0x801DB0D8` on every selected path.
- `s1` advances once at `0x801DB0DC`. Its low-byte bound is 11, and no wrap occurs.

Inductively, after k iterations, the count is between zero and k, and count zero
implies sum zero. The count cannot wrap because k is at most eight. The sum is at
most 524,280, so the 32-bit additions cannot overflow. Even if the later 16-bit
truncation wraps, a nonzero truncated sum still implies at least one selected
iteration.

The branch at `0x801DB0F0` skips division when the low 16 bits of the sum are zero.
Otherwise the divisor is the low byte of a count between one and eight.
Therefore BREAK 7 at `0x801DB104` is unreachable. The numerator is between 1 and
65,535 and the divisor is positive, so the INT_MIN/-1 overflow condition is also
impossible. BREAK 6 at `0x801DB11C` is unreachable.

## Second loop and division

The sum is reset in the delay slot at `0x801DB1A8`; the count and index are reset
at `0x801DB1B4` and `0x801DB1B8`. The loop executes indices zero through two.
The selected value is an unsigned halfword loaded at `0x801DB1E4`. The sum update
is the delay-slot instruction at `0x801DB1F4`; count advances at `0x801DB1FC`.
Both updates share the eligibility branch.

The same induction gives count in zero through three, sum at most 196,605, and
count zero implies sum zero. Division at `0x801DB220` is reached only for a
nonzero low-16-bit sum. The divisor is therefore one through three. BREAK 7 at
`0x801DB22C` and BREAK 6 at `0x801DB244` are unreachable.

## Calls, memory, and entry conditions

The eligibility helper at `0x801DB2F8` is a leaf ending at `0x801DB3A0`. Inspection
of its original instructions shows only reads and operations on `a0`, `at`, `v0`,
and `v1`; it does not modify `s1`, `s2`, or `s3`, write memory, or call another
function. Its return value may vary arbitrarily between iterations without
breaking the induction. The loaded halfword values may also vary arbitrarily.
The proof does not require stable tables, a particular eligibility result, or
nonzero table entries.

No memory store aliases the register-held sum or count in either accumulation
loop. Other helper calls and marking stores occur between loops, before the
second loop's reset, or after its division. They cannot affect the local
induction. Standard function entry at `0x801DB058`, normal return from calls,
and preservation of the executing register context are required. Arbitrary
mid-function entry, corrupted return state, or an interrupt handler that fails
to restore registers is outside ordinary function execution, not a new
caller-value restriction introduced to eliminate the guards.

## IR correspondence and transformation limits

The decoder IR preserves both coupled updates, their branch paths, masks, and
loop bounds. For the first loop, the sum store is based on `%68` and the count
store on `%71`; `%86` masks the divisor to eight bits. The second loop has the
same control-flow structure. All four BREAK calls remain in this pre-optimization
capture. The existing later pipeline already removes the two overflow guards,
but retains the two zero-divisor guards.

The original-byte proof is not itself a plugin transformation. A reusable
elimination must re-establish the coupled-update invariant and loop bound from
its own input, and reject alternate entries, count wrap, independent sum writes,
or unknown effects on the relevant state. It must not identify this case by
name or address. The selected-only decoder represents the helper as a synthetic
`ret undef` body; that placeholder is not callee-effect evidence and must not be
used as proof that an arbitrary call preserves machine state.

## Implemented bounded transformation

The isolated decoder now analyzes little-endian MIPS I leaf helpers directly
from its file image. Each summary visits at most 256 addresses within 1 KiB,
includes branch delay slots and delayed-load destinations, and rejects cycles,
unknown instructions, memory writes, nested calls, and unproven return paths.
It attaches the write mask only after resolved callee addresses are available.
For the eligibility helper the observed mask is `0x0000000e` over 42 instructions,
matching the disassembly's writes to `at`, `v0`, and `v1`.

A decoder-stage interval analysis follows every admitted path from a natural
loop header to a BREAK guard. It seeds only constants established on all incoming
paths, preserves separate path states, and models arithmetic modulo its width.
Unsigned memory loads range over every possible value; they are never assumed
stable. Calls invalidate all registers in their original-byte write mask and
`ra`. Unknown calls, guest-memory stores, nested loops, cross-block expression
SSA, and unsupported control flow abort the proof. Bounds are 64 predecessor
blocks, 4,096 distinct states and 262,144 instruction evaluations per proof.
Budget exhaustion retains the guard. A guard changes only if every state that
reaches it takes the non-BREAK successor.

Both zero-divisor branches become constant-true safe branches in decoder IR.
The later pipeline removes their unreachable blocks and the already redundant
overflow guards. It emits no BREAK dependency for case 053. No generated C was
hand-edited and no runtime binding was added. This proves guard unreachability,
not whole-function runtime equivalence. Completed bounded validation, including
the native exception replay after its approved fixture-only include repair, is
recorded in `acceptance.md`.
