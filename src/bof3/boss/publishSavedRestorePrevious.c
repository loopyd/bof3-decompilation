#include "bof3/bof3.h"

/* Active work-object pointer cell of the scratchpad at 0x1F800044, named
 * g_battle_work by the battle, bmagic, plchar and ui lifts of this project.
 * The original re-materialises the cell address with its own lui for the read
 * and again for each of the two stores, as the exact bmagic sibling
 * publishSavedWorkPointerInvokeReadinessHelperThenForwardBattleByteRaiseFlag4
 * AndInvokeHelperWhenWorkByteBZeroBmagicMagic08103_801EEF88 does, so the cell
 * is reached through this named symbol rather than a folded constant base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM at 0x801EC2E0, outside this overlay
 * image and bound by this target's generated undefined_syms_auto.txt. Word 0x74
 * of that record is the saved work-object pointer: the bmagic lifts at
 * 0x801EEE50, 0x801EEE74 and 0x801EEEDC publish exactly that word into the
 * scratchpad cell and restore the previous pointer after the helper call. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared active-work sequence selector of the main executable at 0x8014D6B8,
 * outside this overlay image and bound by this target's generated
 * undefined_funcs_auto.txt. The AREA030 lifts declare this one-argument form
 * for the same address, and those call sites pass small selectors into it;
 * this body passes 2 and discards the result. */
void func_8014D6B8(s32 arg0);

/* @source 0x800C1A54
 * @behavior Step handler of this overlay work state machine: reads the active
 * work-object pointer at the scratchpad cell 0x1F800044 into a callee-saved
 * register, publishes word 0x74 of the shared battle record 0x801EC2E0, that
 * record's saved work-object pointer, into the same cell, calls the shared
 * active-work sequence selector 0x8014D6B8 with selector 2, restores the cell
 * to the pointer it held, then advances state byte 1 of that previous work
 * object and stores 8 into its word 0xC. The 0x18-byte frame keeps the previous
 * pointer at 0x10 and the return address at 0x14 across the single call; the
 * function writes no other state and takes no arguments. No jal names this
 * span: its address is entry 0 of the thirteen-word code pointer run at
 * 0x800C1804 and the first of the ten entries that
 * dispatchStateByte1Table1804BossBoss05416_800C19C4 copies into its frame
 * table, so the overlay loader that steps the work record reaches it through
 * that run and the epilogue leaves the advanced state byte in $v0 unread.
 * A first seed reached this cell through the SPAD_PTR_SLOT macro and folded
 * one base register across the three accesses, which measured 13/24
 * instructions with a 0x20-byte frame; declaring the cell as a symbol left
 * each access its own lui and measured exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSavedRestorePrevious(
    void) {
  u8 *previous;

  previous = g_battle_work;
  g_battle_work = *(u8 **)(D_801EC2E0 + 0x74);
  func_8014D6B8(2);
  g_battle_work = previous;
  previous[1] = previous[1] + 1;
  *(u32 *)(previous + 0xC) = 8;
}
