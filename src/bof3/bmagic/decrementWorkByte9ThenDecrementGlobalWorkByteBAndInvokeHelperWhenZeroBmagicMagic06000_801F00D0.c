#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This body reads the cell once, so the single access stays one plain pointer
 * load whose address folds into lui/lw. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, and
 * the byte touched here is byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F00D0
 * @behavior Entry index 3 of the six-entry work-byte-2 step table at 0x801F0BBC
 *           that the dispatcher at 0x801EFF88 indexes with the scratchpad work
 *           object's byte at offset 0x2. It reads the work-object pointer cell
 *           at 0x1F800044 once, before the frame is created, decrements that
 *           object's countdown byte at offset 0x09 at byte width (lbu, addiu
 *           -1, sb) and tests the stored byte truncated to eight bits (andi
 *           $v0,$v0,0xFF, no second load of the cell) for zero, so the
 *           truncated register supplies the compared value. Only when the
 *           countdown reaches zero does it read the shared battle work pointer
 *           at 0x801EB4E0, decrement that record's byte at offset 0x0B by one
 *           and invoke the shared helper at 0x801E5988, the store filling the
 *           call delay slot. The 0x18-byte frame keeps only the return address
 *           at 0x10 because of that call. Takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           The 92-byte span is byte-identical to the exact lift at
 *           emi/bmagic/magic131/03@0x801F02B0, proven with one payload
 *           comparison over every byte, and that exact body is the seed here;
 *           this span's own table word, cell addresses and immediates are its
 *           own, and none of them differ. The near-twin at
 *           emi/bmagic/magic151/03@0x801F0CB0 differs in four words (the
 *           decremented offset, its amount, the offset of the second store and
 *           its signed rather than unsigned truncation), so it stayed shape
 *           evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic06000_801F00D0(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
