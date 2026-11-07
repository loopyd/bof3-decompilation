#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises it with
 * `lui $v1,0x1F80` / `lw $v1,68($v1)` once per store group, so each group reads
 * the pointer cell again instead of hoisting a base kept across the stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF3E4
 * @behavior Per-frame step of the work-object handler this overlay dispatches
 *           through entry 5 of the eight-word work-byte-1 handler run at
 *           0x801F0994: it reads the scratchpad work object through the pointer
 *           cell at 0x1F800044 and increments its unsigned byte 0xB. It then
 *           re-reads the cell, decrements byte 9 as a byte-width countdown
 *           (lbu / addiu -1 / sb / andi 0xFF / bnez) and, when the stored byte
 *           is zero, re-reads the cell once more and increments byte 2 so the
 *           dispatcher advances to the next byte-0x2 handler step. The original
 *           materialises the cell address (lui $v1,0x1F80 / lw ...,68($v1))
 *           once per store group, so each group reaches the work object through
 *           the pointer cell instead of a hoisted base. The function is a leaf
 *           with no frame; it takes no arguments, returns nothing and touches
 *           no other state. Its byte-9 countdown group repeats the statement
 *           order and operand forms of
 *           add2ToWorkByteBAdd2ToWorkByteAWhenBelow10ThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic17203_801EF3A4,
 *           which runs the same countdown after an extra byte-0xA step.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndDecrementWorkByte9ThenAdvanceWorkByte2WhenZeroBmagicMagic09903_801EF3E4(void) {
  g_battle_work[0xB] += 1;
  if (--g_battle_work[9] == 0) {
    g_battle_work[2]++;
  }
}
