#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's three separate
 * reads each re-materialise the base. The plain cell is kept rather than the
 * volatile one several siblings use: every read after the first is already
 * separated from the previous one by a store through the loaded pointer, so the
 * reload comes from the store and not from a qualifier. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0C50
 * @behavior Advances the scratchpad work object's byte at offset 0xA by four
 *           modulo 256 through one load of the pointer cell at 0x1F800044
 *           (lbu / addiu 4 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x20 unsigned. Only when that byte is 0x20 or above does it
 *           re-read the pointer cell, store 1 into the object's byte at offset
 *           0xB - that constant is materialised in the delay slot of the bound
 *           test - re-read the pointer cell again and increment the object's
 *           byte at offset 0x2 (lbu / addiu 1 / sb), advancing the overlay work
 *           byte 2 handler sequence. Leaf with no frame; takes no arguments,
 *           returns nothing and reads or writes no other state. Its code pointer
 *           is entry 1 of the overlay handler run at 0x801F24F0, so it is
 *           reached through that table rather than by a call.
 *
 *           The sibling add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103
 *           _801F053C has the same first block and an equality test on a re-read
 *           byte 9; this function tests the stored byte against a bound instead
 *           and inserts byte 0xB before advancing byte 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteAThenSetWorkByteBToOneAndAdvanceWorkByte2WhenAtLeast20BmagicMagic15103_801F0C50(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[0xA] + 4;
    work[0xA] = value;
    if (value >= 0x20) {
        g_battle_work[0xB] = 1;
        g_battle_work[2]++;
    }
}
