#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7960
 * @behavior shop phase step at entry 3 of the phase handler table D_801E545C,
 *           which the phase dispatcher func_801D7344 selects with the UI phase
 *           byte D_80148651: builds the shop slot list through buildSlotList(0),
 *           submits the texture row 0x1C at row 0x15 through func_8014F800 with
 *           the value returned by func_801502D0(0x6D) as its fifth argument,
 *           sets up the field record through func_801D7E1C with the work-block
 *           index byte D_80148656[0], 0x98, 0x3F, 0 and the main-RAM byte
 *           D_8014865D, then draws the panel through
 *           emitFlatLineRectangleBorder with the even value
 *           (index * 54 + 0x3E) & 0xFFFE alongside 0x11, 0x80, 0x34, 0 and 6.
 *           When the two-option prompt helper func_801DA678 reports a choice
 *           through a nonzero low byte, the step charges the selection: it
 *           re-arms the work-block byte D_8014865B with 0xC, the frame timer
 *           phaseTimer with 0x3C and advances the UI phase byte D_80148651 by
 *           two, then subtracts 10 per level of the field record selected by
 *           D_80145FCC[(s8)D_80148656[0] * 0x140] from the main-RAM value
 *           D_80144F50; that level is the +6 byte of the record, reached through
 *           the owning table D_80144968 so the offset folds into the same %lo
 *           immediate the original encodes (the same byte the header declares at
 *           0x8014496E for the first record). When the prompt helper reports a
 *           choice while that selection byte is zero instead, the step plays the
 *           cancel cue 0x106 through func_8015DF18 and rolls the UI phase byte
 *           back by two.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The selection byte is tested through a signed pointer view because the
 * original loads it with `lb` (as the sibling func_801DA678 does at its own
 * signed test) while the same cell is read zero-extended elsewhere.
 *
 * The main-RAM value D_80144F50 is written through a pointer local: the original
 * materializes its address once, before the level load, and both the read and
 * the write of the read-modify-write use that one base at offset 0. The level is
 * read into a local at the original's load position, which precedes the three
 * arming stores.
 */
void drawShopSlotListThenChargeOnConfirm(void) {
  u8* work;
  s32* value;
  s32 record_offset;
  s32 level;

  buildSlotList(0);
  func_8014F800(0x1C, 0x15, 0, 0xFF, func_801502D0(0x6D));
  work = D_80148656;
  func_801D7E1C(work[0], 0x98, 0x3F, 0, D_8014865D);
  emitFlatLineRectangleBorder(0x11, ((s8)work[0] * 54 + 0x3E) & 0xFFFE, 0x80,
                              0x34, 0, 6);
  if ((u8)func_801DA678()) {
    if (*(s8*)&D_8014865B) {
      value = &D_80144F50;
      record_offset = D_80145FCC[(s8)work[0] * 0x140] * 164;
      level = D_80144968[record_offset + 6];
      D_8014865B = 0xC;
      phaseTimer = 0x3C;
      D_80148651 += 2;
      *value -= level * 10;
    } else {
      func_8015DF18(0x106);
      D_80148651 -= 2;
    }
  }
}
