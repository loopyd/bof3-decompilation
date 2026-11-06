#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) once for the byte-9 increment pair
 * and again for the word-0x14 advance, so the cell is reached through this
 * target-map symbol instead of a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF6C8
 * @behavior Per-frame step of a work-object handler dispatched from outside
 * this overlay: reads the scratchpad work object through the pointer cell at
 * 0x1F800044, advances its unsigned byte 0x9 by one (lbu / addiu 1 / sb),
 * re-reads the cell, adds six to its word 0x14 and stores it unconditionally
 * (lw 0x14 / addiu 6 / sw, with the second byte-0x9 read filling the load
 * delay slot), then compares that re-read unsigned byte 0x9 against 0x10 and,
 * when it is equal, increments the object's byte 0x2 with a byte-width add so
 * the handler advances to the next byte-0x2 step. The original is a leaf with
 * no frame; it returns no value and reads or writes no other state. The cell is
 * read twice because the byte-0x9 store in the first group may alias it, and
 * the second read serves the word-0x14 advance and the byte-0x9 test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AdvanceWorkWord14By6AndAdvanceWorkByte2At10BmagicMagic22203_801EF6C8(void) {
    u8 *step;
    u32 *advance;

    step = g_battle_work;
    step[9]++;
    advance = (u32 *)g_battle_work;
    advance[5] += 6;
    if (((u8 *)advance)[9] == 0x10) {
        ((u8 *)advance)[2]++;
    }
}
