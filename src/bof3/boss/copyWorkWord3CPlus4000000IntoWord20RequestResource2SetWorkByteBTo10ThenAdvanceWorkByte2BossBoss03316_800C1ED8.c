#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load; the original re-reads
 * the cell before each later store group because a store through the loaded
 * pointer may alias the cell, so every group names its own local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Resource request helper of the main executable, reached through the generated
 * undefined_funcs_auto row this target already carries because its remaining asm
 * spans call 0x8014D6B8 too. The sibling boss and bmagic lifts declare the same
 * prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* @source 0x800C1ED8
 * @behavior Work-step handler at index 2 of this overlay's code-pointer run
 *           based at 0x800C27AC, which is also the base word 0x800C27B4 that the
 *           step dispatcher func_800C1E70 indexes with work byte 2 after scaling
 *           it by four, so this is the step 0 handler of that byte and it leaves
 *           work byte 2 at step 1. It loads the scratch work object published at
 *           the pointer cell 0x1F800044 and stores that object's word at 0x3C
 *           plus 0x04000000 into its own word at 0x20 (lw / lui 0x400 / addu /
 *           sw, the same literal and operand order the exact sibling
 *           copyRecordTripletMinus4000Plus4000000SetBytes9BTo10AndAToB0Then
 *           AdvanceWorkByte2BmagicMagic07903_801EEE64 uses for the same field),
 *           requests resource 2 through func_8014D6B8, then re-reads the cell to
 *           set object byte 0xB to 0x10 and again to advance object byte 2 by
 *           one. Takes no arguments and returns nothing; reads or writes no
 *           other state. The roles of the words at 0x20 and 0x3C and of byte 0xB
 *           are not established here, so the name describes the stores. No jal
 *           in this payload names 0x800C1ED8; its only in-image reference is the
 *           run word at 0x800C27B4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWord3CPlus4000000IntoWord20RequestResource2SetWorkByteBTo10ThenAdvanceWorkByte2BossBoss03316_800C1ED8(
    void) {
  u8 *work;
  u8 *step;
  u8 *tail;
  u32 value;

  work = D_1F800044;
  value = *(u32 *)(work + 0x3C) + 0x4000000;
  *(u32 *)(work + 0x20) = value;
  func_8014D6B8(2);
  step = D_1F800044;
  step[0xB] = 0x10;
  tail = D_1F800044;
  tail[2]++;
}
