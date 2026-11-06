#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $v0/$a0/$v1,0x1F80 / lw ...,68(...)) once per store group: one
 * load for the byte-0xB add, a second for the guarded byte-0xA add and a third
 * and fourth for the byte-9 countdown and its byte-0x2 arm, so each group
 * reaches the work object through this target symbol instead of a hoisted
 * base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF3A4
 * @behavior Per-frame step of the work-object handler this overlay dispatches
 * through the pointer run at 0x801F0B74: reads the scratchpad work object
 * through the pointer cell at 0x1F800044 and adds two to its unsigned byte
 * 0xB. It then re-reads the cell and adds two to byte 0xA while that byte is
 * below 0x10 (lbu / sltiu 0x10 / beqz, with the add folded into the branch
 * delay slot and the store skipped above the bound). Finally it re-reads the
 * cell, decrements byte 9 as a byte-width countdown (lbu / addiu -1 / sb /
 * andi 0xFF / bnez) and, when the stored byte is zero, re-reads the cell once
 * more and increments byte 2 so the dispatcher advances to the next byte-0x2
 * handler step. The function is a leaf with no frame; it takes no arguments,
 * returns nothing and touches no other state. Its byte-9 countdown group
 * repeats the statement order and operand forms of
 * decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264, which
 * counts down byte 0xA instead of byte 9.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByteBAdd2ToWorkByteAWhenBelow10ThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic17203_801EF3A4(void) {
    g_battle_work[0xB] += 2;
    if (g_battle_work[0xA] < 0x10) {
        g_battle_work[0xA] += 2;
    }
    if (--g_battle_work[9] == 0) {
        g_battle_work[2]++;
    }
}
