#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map row
 * g_battle_work. Declared as the named cell this target's lifted sources use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell before the second access group because the store
 * through the pointer the cell supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF348
 * @behavior Per-frame step of the work object published at the scratchpad
 *           pointer cell 0x1F800044, dispatched as the last of the fourteen
 *           code pointers in the run at 0x801F02A4 (payload offset 0x16A4, inside
 *           this target's reviewed textbin row D_801F02A4) that the overlay entry
 *           selects with work byte 1; no in-image jal reaches this entry. It
 *           increments the object byte 0xA at byte width (lbu $v0,10($v1) /
 *           addiu $v0,$v0,1 / sb $v0,10($v1)), then re-reads the pointer cell with
 *           its own lui $v1,1F80 / lw $v1,68($v1) pair, decrements the object byte
 *           0xB at byte width (lbu / addiu -1 / sb) and compares the truncated
 *           stored value with zero (andi $v0,$v0,0xFF / bnez with an empty delay
 *           slot), so the byte is not re-read. Only when that byte reaches zero it
 *           invokes the shared helper at 0x801E5988, an address outside this
 *           overlay image, in a jal whose delay slot is empty. The 0x18-byte frame
 *           exists only to keep $ra across that call and is created after the
 *           first pointer load, which needs no stack; this entry advances no work
 *           byte 2 and writes no other state. Takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenDecrementWorkByteBAndInvokeHelperWhenZeroBmagicMagic07903_801EF348(void) {
  g_battle_work[0xA]++;
  if (--g_battle_work[0xB] == 0) {
    func_801E5988();
  }
}
