#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, declared as a named fixed address the
 * way this target's exact lifts declare it, so the assembler folds the constant
 * into each access (lui $vN,0x1F80 / lw $vN,0x44($vN)). The original
 * re-materialises the cell before every store group because a store through the
 * pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF678
 * @behavior Per-frame step handler of this overlay's work-byte-2 run: entry 4 of
 *           the eight-word handler table at 0x801EEC1C, the word at 0x801EEC2C,
 *           which is the only reference this payload carries to the span, since
 *           no jal names it. The run dispatcher
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2Set
 *           BmagicMagic02003_801EF3DC copies that table into its own frame and
 *           indexes it with the scratchpad work object's byte 2, so this body
 *           runs while work byte 2 is 4. It decrements the work object's
 *           unsigned countdown byte 9 in place (lbu / addiu -1 / sb / andi 0xFF /
 *           bnez), so the rest of the body runs only when that countdown reaches
 *           zero. On zero it re-reads the cell and raises bit 0x20 of work byte
 *           0, re-reads it again and sets mode byte 0x5C to 1, then sets the
 *           signed work colour triplet at 0x5D, 0x5E and 0x5F to -0x40, which is
 *           0xC0 at byte width, from one materialised constant reused across the
 *           three stores and one cell read per channel. Last it re-reads the cell
 *           and advances work byte 2 by one with a byte-width add, moving the run
 *           to entry 5. The neighbouring fade steps of this run move the same
 *           triplet: subtractWorkColorTripletBy8ThenCopyRecordTripletAndAdvance
 *           WorkByte2WhenChannelIs80BmagicMagic02003_801EF720 subtracts 0x8 per
 *           frame from 0x5D/0x5E/0x5F and tests 0x5D at 0x80, and addWorkColor
 *           TripletBy16ThenSetGlobalWorkByteBToFFAndAdvanceWorkByte2WhenChannel
 *           IsZeroBmagicMagic02003_801EF7D0 adds 0x10, so this handler seeds the
 *           0xC0 those steps move from. Leaf with no stack frame; takes no
 *           arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSetFlag20Mode1SeedWorkColorTripletToC0AndAdvanceWorkByte2BmagicMagic02003_801EF678(void) {
  if (--D_1F800044[9] == 0) {
    D_1F800044[0] |= 0x20;
    D_1F800044[0x5C] = 1;
    ((s8 *)D_1F800044)[0x5D] = -0x40;
    ((s8 *)D_1F800044)[0x5E] = -0x40;
    ((s8 *)D_1F800044)[0x5F] = -0x40;
    D_1F800044[2]++;
  }
}
