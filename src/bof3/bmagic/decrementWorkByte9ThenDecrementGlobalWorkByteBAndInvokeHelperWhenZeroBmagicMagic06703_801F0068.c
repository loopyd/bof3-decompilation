#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The body reads the cell once, so the access stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F0068
 * @behavior Decrements the scratchpad work object's byte 0x9 at byte width
 *           (lbu / addiu -1 / sb) and tests the stored value truncated to eight
 *           bits (andi $v0,$v0,0xFF / bnez), so the truncated register, not a
 *           reload, supplies the compared value. Only when that byte reaches
 *           zero does it load the shared battle work-record pointer at
 *           0x801EB4E0, decrement that record's byte 0xB by one and invoke the
 *           shared helper at 0x801E5988, the byte store filling the call delay
 *           slot. The pointer-cell load precedes frame creation; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call. The payload holds one
 *           word naming this address, at 0x801F04E4: index 3 of the pointer run
 *           based at 0x801F04D8 that the work-byte-2 dispatcher at 0x801EFF24
 *           indexes, and no jal immediate reaches it. Takes no arguments,
 *           returns nothing and reads or writes no other state. The captured
 *           0x5C bytes are byte-identical to the exact body at
 *           emi/bmagic/magic131/03 0x801F02B0 (payload comparison of 92 bytes),
 *           which supplied the statement order and operand forms; literals,
 *           offsets and cell bindings are this target's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic06703_801F0068(void) {
  if (--g_battle_work[0x9] == 0) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
