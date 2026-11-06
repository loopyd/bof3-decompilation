#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's three separate
 * reads each re-materialise the base. The plain cell is kept rather than the
 * volatile one several siblings use: every read after the first is already
 * separated from the previous one by a store through the loaded pointer, so the
 * reload comes from the store and not from a qualifier, and the sibling
 * seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90 measured the volatile spelling identical on this
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F129C
 * @behavior Per-frame step of the overlay work object, reached through the
 *           code pointer run at 0x801F24A4 rather than by a call: it reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and increments
 *           the object's byte at offset 0xB by one (lbu / addiu 1 / sb), then
 *           re-reads the cell and adds four to the object's byte at offset 0xA
 *           (lbu / addiu 4 / sb). It then re-reads the cell a third time and
 *           tests the object's byte at offset 0xA against 0x10 by equality
 *           (lbu / addiu $v0,$zero,0x10 / bne); only when that byte equals 0x10
 *           does it store 8 into the object's byte at offset 0x9 - the constant
 *           is materialised in the delay slot of the equality test and the
 *           store goes through the pointer loaded for the test - and re-read
 *           the cell to increment the object's state byte at offset 0x2 (lbu /
 *           addiu 1 / sb), advancing the overlay work byte 2 handler sequence.
 *           Leaf with no frame; takes no arguments, returns nothing and reads
 *           or writes no other state.
 *
 *           Shape: the first two statements each take one cell read, as in the
 *           exact sibling add4ToWorkByteAThenSetWorkByteBToOneAndAdvanceWork
 *           Byte2WhenAtLeast20BmagicMagic15103_801F0C50, which adds four to the
 *           same work byte 0xA. That sibling tests the byte it just stored;
 *           this original reads the cell again and compares the re-read byte
 *           against 0x10, the shape of the 13103 sibling
 *           add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103_801F053C
 *           (store through one cell read, reload, compare the re-read byte to a
 *           literal). The arm then stores 8 through the pointer the equality
 *           test loaded - the original materialises no second cell base before
 *           that store - and only the trailing byte 2 increment re-reads the
 *           cell, the reload-in-arm-only shape of
 *           decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2
 *           WhenZeroBmagicMagic15103_801EFD24.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAdd4ToWorkByteAThenSetWorkByte9To8AndAdvanceWorkByte2At10BmagicMagic15103_801F129C(void) {
    u8 *work;

    g_battle_work[0xB]++;
    g_battle_work[0xA] += 4;
    work = g_battle_work;
    if (work[0xA] == 0x10) {
        work[9] = 8;
        g_battle_work[2]++;
    }
}
