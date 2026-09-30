#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * sibling lift incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4
 * reads the same cell, and the sibling battle lifts own the battle work record
 * (src/bof3/battle/func_801E8558_battle03.c reads it, runQueuedSlotHandlers.c
 * assigns it). Splat already binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFDC0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and, only when the work object's byte at offset 0xB is zero, stores 2 into
 * the byte at offset 0xB of the shared battle work record pointed to by
 * 0x801EB4E0 - the original materializes the constant (addiu $v0,$zero,2) and
 * issues the sb in the delay slot of the helper call - and then invokes the
 * shared helper at 0x801E5988. Nothing is returned and the 0x18-byte frame
 * only keeps $ra across that call, so no other state is read or written. The
 * guard byte and the stored byte belong to different records: the guard is the
 * scratchpad work-object byte 0xB (the same guard as the sibling
 * incrementWorkByte2WhenWorkByteBZeroBmagicMagic15103_801F00B4), the store
 * target is the shared battle work record's byte 0xB.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setGlobalWorkByteBTo2ThenInvokeHelperWhenWorkByteBZeroBmagicMagic15103_801EFDC0(
    void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    D_801EB4E0[0xB] = 2;
    func_801E5988();
  }
}
