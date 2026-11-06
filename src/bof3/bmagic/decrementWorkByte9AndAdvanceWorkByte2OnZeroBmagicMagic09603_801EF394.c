#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF394
 * @behavior Decrements the unsigned byte 9 of the scratchpad work object
 * reached through the pointer cell at 0x1F800044 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and compares the stored value with
 * zero (andi 0xFF / bnez). Only when that byte reaches zero it re-reads the
 * cell and increments the object's byte 2 with a byte-width add, advancing the
 * dispatcher on work byte 2 to its next handler step. The second read of the
 * cell is required: the store through the loaded work pointer may alias the
 * cell. The function is a leaf with no frame; it takes no arguments, returns
 * nothing and touches no other state. Bytes 0x801EF394..0x801EF3DF are
 * byte-identical to the byte-9 countdown tail group of the exact sibling
 * add2ToWorkByteBAdd2ToWorkByteAWhenBelow10ThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic17203_801EF3A4
 * and differ from decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264,
 * whose statement order, operand forms and declaration style this body
 * follows, only in the counted-down offset (9 instead of 0xA).
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF394(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[2]++;
    }
}
