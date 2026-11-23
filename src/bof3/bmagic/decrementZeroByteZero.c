#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044; this target's map binds the
 * address as g_battle_work, the name its other exact lifts use. A byte store
 * through the loaded pointer may alias the cell, so the original reads the cell
 * again before the following byte read-modify-write. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEFE0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and returns immediately when the work object's byte at offset 0xB
 *           is non-zero (lbu, bnez with an empty delay slot). Otherwise it
 *           decrements work byte 9 at byte width (lbu, addiu -1, sb, andi 0xFF)
 *           and returns when the stored byte is non-zero. When that byte
 *           reaches zero it re-reads the pointer cell and increments work byte
 *           1 by one at byte width (lbu, addiu 1, sb), advancing the work-byte-1
 *           step index that this overlay's dispatcher func_801EEC38 uses. Entry
 *           2 (index 2) of the four-word code pointer run at 0x801EEC04
 *           (0x801EED8C, 0x801EEEB0, 0x801EEFE0, 0x801EF03C), which that
 *           dispatcher copies into its own frame and calls with no arguments,
 *           so no jal word in the payload reaches this span: an overlay
 *           dispatch entry, not a called function. Takes no arguments, returns
 *           nothing, writes no other state, and the leaf body creates no frame.
 *           Shape: statement order, operand forms and declaration style follow
 *           two exact lifts. The head comes from emi/bmagic/magic067/03 at
 *           0x801EEFA4, which reads the same cell and guards on the same
 *           lbu/bnez test of work byte 0xB before advancing work byte 1; the
 *           countdown test comes from emi/bmagic/magic130/03 at 0x801EEE0C,
 *           which decrements work byte 9 at byte width and advances work byte 1
 *           on the zero result. Neither sibling carries this body, so the
 *           guarded tail was written from this span's own bytes: no call, no
 *           flag raise, and one advance of work byte 1 after the countdown.
 *           A search for these 92 payload bytes across out/binaries returned
 *           only this target's own span at 0x3E0, so no byte-identical twin
 *           exists and the clone route was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementZeroByteZero(
    void) {
    if (g_battle_work[0xB] == 0) {
        if (--g_battle_work[9] == 0) {
            g_battle_work[1]++;
        }
    }
}
