#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF194
 * @behavior Decrements the unsigned byte 9 of the scratchpad work object
 * reached through the pointer cell at 0x1F800044 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and compares the stored value with
 * zero (andi 0xFF / bnez). Only when that byte reaches zero it re-reads the
 * cell and increments the object's byte 2 with a byte-width add, advancing the
 * dispatcher on work byte 2 to its next handler step. The second read of the
 * cell is required: the store through the loaded work pointer may alias the
 * cell. The function is a leaf with no frame; it takes no arguments, returns
 * nothing and touches no other state.
 * The span 0x801EF194..0x801EF1DF (0x4C bytes) is byte-identical to
 * 0x801EF394..0x801EF3DF in emi/bmagic/magic096/03, whose exact lift
 * decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF394 is the
 * representative this body is cloned from; it keeps the same statement order,
 * operand forms and declaration style, so the scratchpad cell keeps the map
 * spelling g_battle_work instead of a second name at the same address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic13003_801EF194(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[2]++;
    }
}
