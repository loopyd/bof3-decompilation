#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This body reads the cell twice, so it keeps the named cell and each access
 * materialises its own lui/lw pair rather than a constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, and
 * the byte touched here is byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F03BC
 * @behavior Last word (index 10) of the work-object byte-1 handler table based
 *           at 0x801F0BB0, reached through the pointer word at 0x801F0BD8 that
 *           the dispatcher at 0x801EFF44 loads with the work-object byte at
 *           offset 0x1 scaled left 2, so nothing calls it directly. It reads
 *           the scratchpad work-object pointer cell at 0x1F800044 once before
 *           the frame is created, subtracts two from that object's byte 0x09 at
 *           byte width (lbu, addiu -0x2, sb), then reads the cell a second time
 *           (a fresh lui/lw pair) and tests the reloaded byte for zero, so the
 *           pointer is re-materialised rather than reused and no register
 *           truncation follows the store. Only when that countdown reaches zero
 *           does it read the shared battle work pointer at 0x801EB4E0,
 *           decrement that record's byte 0x0B by one and invoke the shared
 *           helper at 0x801E5988, the store filling the call delay slot. The
 *           0x18-byte frame keeps only the return address at 0x10 because of
 *           that call. Takes no arguments, returns nothing through the jr $ra
 *           at 0x801F0420 and reads or writes no other state; the roles of work
 *           byte 0x09 and record byte 0x0B are not established, so they stay
 *           offset-anchored.
 *
 *           Shape: this target's exact lift at 0x801F00D0 carries the same
 *           statement order, operand forms and declaration style but tests the
 *           stored register (addiu -0x1, andi 0xFF, bnez) instead of re-reading
 *           the cell, so it seeded this body while the two spans compare
 *           unequal; the immediate -0x2 and the reloading test are this span's
 *           own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic06000_801F03BC(void) {
  g_battle_work[0x09] -= 2;

  if (g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
