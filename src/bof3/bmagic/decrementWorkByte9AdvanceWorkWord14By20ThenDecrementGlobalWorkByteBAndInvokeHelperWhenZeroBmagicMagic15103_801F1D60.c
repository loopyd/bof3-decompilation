#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * no target-map row is needed. The exact siblings
 * decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F0CB0
 * and setGlobalWorkByteBTo2ThenInvokeHelperWhenWorkByteBZeroBmagicMagic15103_801EFDC0
 * reach the same record byte 0xB and the same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F1D60
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once and
 *           performs two read-modify-writes through that single loaded pointer:
 *           it decrements the work-object byte at offset 0x9 at byte width
 *           (lbu / addiu -1 / sb) and advances the work-object word at offset
 *           0x14 by 0x20, whose store the original leaves for the branch delay
 *           slot. The byte the original compares is the stored register
 *           truncated to eight bits (andi $v1,$v1,0xFF), so no second cell read
 *           supplies the guard. Only when that byte reaches zero does it read
 *           the shared battle work record pointer 0x801EB4E0, decrement that
 *           record byte 0xB by one and invoke the shared helper at 0x801E5988,
 *           the store filling the call delay slot. Frame 0x18 with $ra only,
 *           because of the call; the cell load precedes frame creation because
 *           the decrement needs no stack. Takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           The predecrement spelling carries the shape: a named `u8`
 *           accumulator for the byte measured 17/24 at the same size, with the
 *           pointer and the word value holding each other's registers from the
 *           first instruction. The accumulator declaration is what flips that
 *           allocation, so keep the byte update in place on the cell expression.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AdvanceWorkWord14By20ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F1D60(void) {
    u8 *work;
    s32 value;

    work = SPAD_PTR_SLOT(u8, 0x44u);
    value = *(s32 *)(work + 0x14) + 0x20;
    *(s32 *)(work + 0x14) = value;
    if (--work[0x9] == 0) {
        u8 *object;

        object = D_801EB4E0;
        object[0xB]--;
        func_801E5988();
    }
}
