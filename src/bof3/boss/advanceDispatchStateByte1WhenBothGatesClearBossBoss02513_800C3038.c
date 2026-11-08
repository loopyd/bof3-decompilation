#include "bof3/bof3.h"

/* The scratchpad work-object pointer cell 0x1F800044 every overlay shares, and
 * the two gate cells this overlay's step handlers read. Both gates appear with
 * the same two loads in the next handler of this overlay's run, 0x800C3088,
 * which decrements the same work byte when either gate is set; here both gate
 * loads must answer zero before the work byte moves. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80148603; /* @source 0x80148603 @kind unknown */
extern u8 D_80146329; /* @source 0x80146329 @kind unknown */

/* @source 0x800C3038
 * @behavior Fourth word of the five-entry handler run at 0x800C3294, the words
 *           0x800C2128, 0x800C2160, 0x800C217C, 0x800C3038 and 0x800C3088.
 *           The sibling dispatcher 0x800C2FF4 indexes the run's fourth word as
 *           its base with work-record byte 1, so this handler runs when that
 *           dispatch state byte holds zero. It reads the global byte at
 *           0x80148603 and returns without writing anything when that byte is
 *           non-zero, then reads the global byte at 0x80146329 and returns
 *           when bit 0x2 of it is set. Only with both gates clear does it load
 *           the scratchpad work-object pointer cell 0x1F800044 once and
 *           increment the work object's byte at offset 1 by one at byte width,
 *           so the dispatch state byte moves from zero to one and the next
 *           dispatch reaches the neighbouring handler 0x800C3088. It takes no
 *           arguments, keeps no frame and touches no other cell, so nothing is
 *           returned. That neighbour tests the same two gate cells and
 *           decrements the same work byte when either gate is set, so the two
 *           handlers move the counter in opposite directions; only the
 *           increment constant, the store order and the branch sides differ,
 *           which makes that unmatching sibling shape evidence rather than a
 *           clone. The statement order, the incremented reference and the
 *           single cell load follow the exact work-byte advances of the bmagic
 *           overlays, for example
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBIs7BmagicMagic10403_801EEE7C.
 *           The run word at 0x800C32A0 is the only in-image word equal to this
 *           address and no jal in the payload reaches it, so the dispatcher is
 *           the shape of its caller, not a missing one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceDispatchStateByte1WhenBothGatesClearBossBoss02513_800C3038(void) {
  if (D_80148603 == 0 && (D_80146329 & 0x02) == 0) {
    g_battle_work[1]++;
  }
}
