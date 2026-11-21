#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF348
 * @behavior Decrements the unsigned byte 11 of the scratchpad work object
 * reached through the pointer cell at 0x1F800044 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and compares the stored value with
 * zero (andi 0xFF / bnez). Only when that byte reaches zero it re-reads the
 * cell and increments the object's byte 2 with a byte-width add, advancing the
 * dispatcher on work byte 2 to its next handler step. The second read of the
 * cell is required: the store through the loaded work pointer may alias the
 * cell. The function is a leaf with no frame; it takes no arguments, returns
 * nothing and touches no other state. Its 19-instruction body differs from the
 * payload span at 0x801EF394, whose statement order, operand forms, offset
 * spelling and declaration style this body follows, only in the two byte-offset
 * immediates it carries: the counted-down byte is 11 instead of 9.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBAndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF348(void) {
    if (--g_battle_work[11] == 0) {
        g_battle_work[2]++;
    }
}
