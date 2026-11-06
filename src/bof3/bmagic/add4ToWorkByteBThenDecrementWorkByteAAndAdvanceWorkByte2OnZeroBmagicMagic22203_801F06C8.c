#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $v1,0x1F80 / lw $v1,68($v1)) once for the byte-0xB add and again
 * for the byte-0xA countdown and its byte-0x2 arm, so each group reaches the
 * work object through this target-map symbol instead of a hoisted base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F06C8
 * @behavior Per-frame step of a work-object handler this overlay dispatches on
 * a work-object byte: reads the scratchpad work object through the pointer cell
 * at 0x1F800044 and adds four to its unsigned byte 0xB with a byte-width
 * add-then-store (lbu / addiu 4 / sb). It then re-reads the cell and
 * decrements byte 0xA in place, comparing the stored byte with zero
 * (sb / andi 0xFF / bnez); when that byte is zero it re-reads the cell once
 * more and increments the object's byte 0x2 with a byte-width add so the
 * dispatcher advances to the next byte-0x2 handler step. The function is a
 * leaf with no frame; it returns nothing and reads or writes no other state.
 * Its trailing 0x4C bytes are byte-identical to
 * decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic13103_801F0264, whose
 * exact source that part follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteBThenDecrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic22203_801F06C8(void) {
    g_battle_work[0xB] += 4;
    if (--g_battle_work[0xA] == 0) {
        g_battle_work[2]++;
    }
}
