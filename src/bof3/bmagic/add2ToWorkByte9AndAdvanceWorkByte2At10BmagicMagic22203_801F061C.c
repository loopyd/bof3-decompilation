#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $v0,0x1F80 / lw $v0,68($v0)) for the increment's load and store
 * pair and again for the byte-2 arm, so the cell is a plain pointer like its
 * byte-identical sibling add2ToWorkByte9AndAdvanceWorkByte2At10
 * BmagicMagic13103_801F01A8. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F061C
 * @behavior Per-frame step of a work-object handler this overlay dispatches
 * on a work-object byte: reads the scratchpad work object through the pointer
 * cell at 0x1F800044 once, loads its unsigned byte 0x9 and advances it by two
 * with a byte-width add (lbu / addiu 2 / sb). When the new byte 0x9 equals
 * 0x10 it re-reads the cell and increments the object's byte 0x2 with a
 * byte-width add (lbu / addiu 1 / sb), advancing to the next byte-0x2 handler.
 * The function is a leaf with no frame; it returns nothing and reads or writes
 * no other state. Bytes 0x801F061C..0x801F066B are byte-identical to
 * 0x801F01A8..0x801F01F7 of the magic131/03 overlay, whose exact source this
 * body follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic22203_801F061C(void) {
    if ((g_battle_work[9] += 2) == 0x10) {
        g_battle_work[2]++;
    }
}
