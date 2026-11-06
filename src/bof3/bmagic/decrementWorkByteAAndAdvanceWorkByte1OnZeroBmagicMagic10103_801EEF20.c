#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of this handler in the bmagic overlay family. It
 * stays a named cell rather than the constant-address memory accessor because
 * the original re-materialises the base at each access (lui $v1,0x1F80 /
 * lw $v1,68($v1)): the accessor's constant address is folded once by CSE into a
 * full-address register and reused, which loads the cell through that one base
 * instead of reloading it after the first store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEF20
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. Decrements the object's unsigned byte 0xA
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb) and
 *           compares the stored value with zero (andi 0xFF / bnez). Only when
 *           that byte reaches zero it re-reads the cell and increments the
 *           object's byte 1 by one at byte width, which is the state byte the
 *           overlay's step dispatcher at 0x801EF310 reads to index the handler
 *           table D_801EFCBC. The second read of the cell is required: the store
 *           through the loaded work pointer may alias the cell. The function is
 *           a leaf with no frame; it takes no arguments, returns nothing and
 *           touches no other state. The only in-image reference to 0x801EEF20 is
 *           the word at payload offset 0x10, the third entry of this overlay's
 *           leading five-word handler run (0x801EECCC, 0x801EEE68, 0x801EEED4,
 *           0x801EEF20, 0x801EEF6C), so the entry is dispatched and has no jal
 *           caller. Statement order, operand forms and declaration style follow
 *           the exact sibling
 *           decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF394,
 *           whose bytes differ only in the two byte offsets (9/2 there, 0xA/1
 *           here); no byte-identical copy of this span exists in any other
 *           bmagic overlay payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte1OnZeroBmagicMagic10103_801EEF20(void) {
    if (--g_battle_work[0xA] == 0) {
        g_battle_work[1]++;
    }
}
