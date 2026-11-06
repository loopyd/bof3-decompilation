#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)). The plain cell is kept rather
 * than the volatile one several siblings use: this function reads the cell once
 * and every later access goes through that loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * no target-map row is needed. The exact sibling
 * setGlobalWorkByteBTo2ThenInvokeHelperWhenWorkByteBZeroBmagicMagic15103_801EFDC0
 * stores through the same record byte 0xB and reaches the same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0CB0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           decrements the object's tint channel byte at offset 0x5D by two with
 *           a byte-width read-modify-write (lbu / addiu -2 / sb), truncates the
 *           value it just stored to the byte it wrote (sll 24, no second load of
 *           the cell) and tests that byte for zero. Only when the channel
 *           reaches zero does it read the shared battle work record pointer
 *           0x801EB4E0, decrement that record's byte 0xB by one and invoke the
 *           shared helper at 0x801E5988, the store filling the call delay slot.
 *           Frame 0x18 with $ra only, because of the call; nothing is returned
 *           and no other state is read or written.
 *
 *           The channel is carried in an s8 local: the original truncates the
 *           stored byte before the zero test, which is the signed byte view, and
 *           the exact sibling
 *           decrementWorkTintChannel5DThenIncrementWorkByte2WhenChannelIs8
 *           BmagicMagic15103_801F00E8 compares the same channel through a
 *           signed view. A u8 accumulator, with or without an (s8) cast at the
 *           test, leaves a different truncation in its place.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F0CB0(void) {
    u8 *work;
    s8 channel;

    work = g_battle_work;
    channel = work[0x5D] - 2;
    work[0x5D] = channel;
    if (channel == 0) {
        u8 *object;

        object = D_801EB4E0;
        object[0xB]--;
        func_801E5988();
    }
}
