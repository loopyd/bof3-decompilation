#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE640
 * @behavior increments battle work byte +0x0A while it is below 8; then
 * increments work byte +0x09, adds 2 to signed counter +0x10 and accumulates
 * that counter into halfword +0x30; when byte +0x09 reaches 0x0C it resets
 * counter +0x10 to -6 and increments work byte +0x01.
 * @status partial
 * @match 55.88
 * @residual first mismatch +0x0000: the work pointer is allocated to $a1 and
 *   the +0x09 counter to $a0, the reverse of the original. Every one of the 15
 *   differing instructions is that single swap, both bodies are 136 bytes, and
 *   the reset arm already matches. Residual class: allocator choice between a
 *   block-local counter pseudo and a cross-block pointer pseudo. Repair round:
 *   hoisting the post-if reload out of the if body, so the pointer's second
 *   value stops crossing the join, measures 18/34 at 136 bytes and still
 *   allocates $a1, which refutes that explanation, and it also makes the reload
 *   unconditional where the original reloads only on the taken path. Three
 *   declaration-order permutations of the same statements all stay at 19/34,
 *   and moving the reset arm's `u8 *current;` to function scope (declaration
 *   scope, the last untried numbering dimension) is also inert at 19/34.
 *   Compiler-profile and permuter rungs are not authorized, so they stay untried
 *   rather than exhausted.
 */
void advanceStageAndAccumulateWorkCounter(void)
{
    BattleWork *work;
    s32 value;
    u8 counter;
    u16 total;

    work = (BattleWork *)g_battle_work;
    if (work->unk_09[1] < 8) {
        work->unk_09[1] = work->unk_09[1] + 1;
        work = (BattleWork *)g_battle_work;
    }
    counter = work->unk_09[0] + 1;
    value = work->unk_10 + 2;
    total = work->unk_30 + value;
    work->unk_09[0] = counter;
    work->unk_10 = value;
    work->unk_30 = total;
    if (counter == 0x0C) {
        u8 *current;

        current = g_battle_work;
        ((BattleWork *)current)->unk_10 = -6;
        current[1] = current[1] + 1;
    }
}
