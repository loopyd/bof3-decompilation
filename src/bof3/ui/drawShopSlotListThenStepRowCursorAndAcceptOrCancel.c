#include "bof3/ui/shop00_internal.h"

/* @source 0x801D76B8
 * @behavior shop phase step at entry 2 of the phase handler table D_801E545C,
 *           which the phase dispatcher func_801D7344 selects with the UI phase
 *           byte D_80148651: it rebuilds the shop slot list through
 *           buildSlotList(0), submits the texture row 0x1C at row 0x15 through
 *           func_8014F800 with the halfword handle
 *           func_801502D0((s8)D_8014865D + 0x65) as its fifth argument, draws
 *           the slot price list through drawShopSlotPriceList with the
 *           work-block index byte D_80148656[0], 0x98, 0x3F, 0 and the 0xFF
 *           no-row mask, draws the panel through emitFlatLineRectangleBorder
 *           with the even value (index * 54 + 0x3E) & 0xFFFE alongside 0x11,
 *           0x80, 0x34, 0 and 6, and marks the row D_8014865D through
 *           func_801647C4 at the fixed column 0x9B, the halfword row offset
 *           (row * 13 + 0x44) & 0xFFFF and the mode 0. It then reads the pad
 *           direction func_8014E640 returns for the mask 0x5000 of the pad
 *           state D_80145AA8 and steps that row byte with it: bit 0x4000 plays
 *           cue 0x100 and increments the byte, wrapping it back to zero once
 *           the incremented byte is no longer below 8 as a signed byte, and
 *           bit 0x1000 plays cue 0x100 and decrements the byte, wrapping it to
 *           7 once the decremented byte goes negative. When the pad state then
 *           shares a bit with the confirm mask D_80145AC2 the step dispatches
 *           on the price of the 164-byte field record that
 *           D_80145FCC[(s8)D_80148656[0] * 0x140] selects: while the main-RAM
 *           value D_80144F50 is below ten times that record's +6 byte it plays
 *           cue 0x107, arms the work-block byte D_8014865B with 0xB, the
 *           saved-state byte D_8014865F with 0xC and the frame timer
 *           phaseTimer with 0x3C and advances the UI phase byte by five,
 *           otherwise it plays cue 0x104, clears D_8014865B and advances the UI
 *           phase byte by one. When the pad state shares a bit with the cancel
 *           mask D_80145AC4 instead it plays cue 0x106 and rolls the UI phase
 *           byte back by one.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The row byte is reached through the global at every site: the original
 * materialises its address once into $s1 for the reads that straddle the draw
 * calls and again, from scratch, in the decrement arm. Spelling the decrement
 * arm with the same live pointer as the rest of the body scored 165 of 170
 * instructions with the first difference on that arm's reload. The handle
 * argument of func_801502D0 is truncated to a halfword, which is why the
 * original masks the sign-extended byte sum with 0xFFFF.
 *
 * The work-block base byte is read three times: zero-extended for the price
 * list index and signed for the panel offset and for the record selector. The
 * unsigned read takes the block base through its mapped neighbour
 * (D_80148655 + 1) while the two signed reads index the array view, because
 * spelling all three through that array made gcc keep one base in a
 * callee-saved register and reuse it for both loads; that candidate measured
 * 165 of 170 instructions with the first difference on the base. The price
 * record is reached through the owning table D_80144968 at offset +6, so the
 * original folds that offset into the same %lo immediate the header declares at
 * 0x8014496E, and the money comparison is the unsigned sltu of (u32)D_80144F50
 * against ten times the level byte.
 */
void drawShopSlotListThenStepRowCursorAndAcceptOrCancel(void) {
  u8* row;
  u8* cursor;
  s32 direction;
  s32 record_offset;

  buildSlotList(0);
  row = &D_8014865D;
  func_8014F800(0x1C, 0x15, 0, 0xFF,
                func_801502D0((u16)((s8)*row + 0x65)));
  drawShopSlotPriceList(*(&D_80148655 + 1), 0x98, 0x3F, 0, 0xFF);
  emitFlatLineRectangleBorder(0x11, ((s8)D_80148656[0] * 54 + 0x3E) & 0xFFFE,
                              0x80, 0x34, 0, 6);
  func_801647C4(0x9B, (u16)((s8)*row * 13 + 0x44), 0);

  direction = func_8014E640(D_80145AA8 & 0x5000);
  if (direction & 0x4000) {
    func_8015DF18(0x100);
    if ((s8)(++*row) >= 8) {
      *row = 0;
    }
  }
  if (direction & 0x1000) {
    func_8015DF18(0x100);
    cursor = &D_8014865D;
    if ((s8)(--*cursor) < 0) {
      *cursor = 7;
    }
  }

  if (D_80145AA8 & D_80145AC2) {
    record_offset = D_80145FCC[(s8)D_80148656[0] * 0x140] * 164;
    if ((u32)D_80144F50 < D_80144968[record_offset + 6] * 10) {
      func_8015DF18(0x107);
      D_8014865B = 0xB;
      D_8014865F = 0xC;
      phaseTimer = 0x3C;
      D_80148651 += 5;
    } else {
      func_8015DF18(0x104);
      D_8014865B = 0;
      D_80148651 += 1;
    }
  }
  if (D_80145AA8 & D_80145AC4) {
    u8* phase;

    func_8015DF18(0x106);
    phase = &D_80148651;
    *phase -= 1;
  }
}
