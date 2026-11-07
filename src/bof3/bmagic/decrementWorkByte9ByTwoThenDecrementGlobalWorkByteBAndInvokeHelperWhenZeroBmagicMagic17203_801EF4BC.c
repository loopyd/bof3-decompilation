#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This body reads the cell once and both the byte load and the byte store go
 * through that one loaded pointer, so the access stays one plain cell load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt binds the address, so no
 * target-map row is needed; the exact sibling
 * decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenZeroBmagicMagic15103_801F0CB0 reads the same record word and decrements
 * the same byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF4BC
 * @behavior Per-call step of the work-object handler this overlay dispatches
 *           through the pointer run at 0x801F0B74: reads the scratchpad work
 *           object through the pointer cell at 0x1F800044, subtracts two from
 *           its countdown byte 9 at byte width (lbu / addiu -2 / sb) and tests
 *           the stored byte truncated to eight bits (andi $v0,$v0,0xFF /
 *           bnez), so the truncated register - not a reload - supplies the
 *           compared value. Only when the countdown reaches zero does it read
 *           the shared battle work record pointer at 0x801EB4E0, decrement that
 *           record's byte 0xB by one and invoke the shared helper at
 *           0x801E5988, the store filling the call delay slot. The pointer-cell
 *           load precedes frame creation; the 0x18-byte frame only keeps $ra at
 *           0x10 across the call. Takes no arguments, returns nothing and reads
 *           or writes no other state.
 *
 *           The 92-byte body is byte-identical to the still-asm span at
 *           emi/bmagic/magic111/03 0x801EF914 (payload offset 3348), proven with
 *           one payload comparison over every out/binaries payload; that row has
 *           no authored C yet, so the clone lever offered no source and this
 *           body was seeded from the sibling shape of
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhen
 *           ZeroBmagicMagic13103_801F02B0, which counts byte 9 down by one and
 *           otherwise repeats this statement order and operand forms.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic17203_801EF4BC(void) {
    if ((g_battle_work[9] -= 2) == 0) {
        D_801EB4E0[0xB] -= 1;
        func_801E5988();
    }
}
