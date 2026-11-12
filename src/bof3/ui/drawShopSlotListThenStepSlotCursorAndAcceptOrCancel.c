#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7444
 * @behavior shop phase step at entry 1 of the phase handler table D_801E545C,
 *           which the phase dispatcher func_801D7344 selects with the UI phase
 *           byte D_80148651: it reads the visible slot count through
 *           func_801BDB7C(0), rebuilds the slot list through buildSlotList(0),
 *           submits the texture row 0x1C at row 0x15 through func_8014F800 with
 *           the value returned by func_801502D0(0x63) as its fifth argument,
 *           draws the slot price list through drawShopSlotPriceList with the
 *           work-block index byte D_80148656[0], 0x98, 0x3F, 0 and 0xFF, and
 *           marks the selected row through emitFlatLineRectangleBorder with the
 *           even value (index * 54 + 0x3E) & 0xFFFE alongside 0x11, 0x80, 0x34,
 *           1 and 6. It then reads the pad direction func_8014E640 returns for
 *           the mask 0x5000 of the pad state D_80145AA8 and steps the
 *           work-block index byte with it: bit 0x4000 plays cue 0x100 and
 *           increments the index, wrapping it back to zero once the
 *           incremented byte is no longer below the slot count as a signed
 *           byte, and bit 0x1000 plays cue 0x100 and decrements the index,
 *           wrapping it to the count minus one once the decremented byte goes
 *           negative. When the pad state then shares a bit with the confirm
 *           mask D_80145AC2 the step dispatches on the +0x19 bit-mask byte of
 *           the field record D_80145FCC[(s8)index * 0x140] selects: a nonzero
 *           mask plays cue 0x107, arms the work-block byte D_8014865B with 1,
 *           the saved-state byte D_8014865F with 2 and the frame timer
 *           phaseTimer with 0x3C and advances the UI phase byte by six,
 *           while a zero mask plays cue 0x104, clears the main-RAM byte
 *           D_8014865D and advances the UI phase byte by one. When the pad
 *           state shares a bit with the cancel mask D_80145AC4 instead it
 *           plays cue 0x106, arms D_8014865B with 0xF, sets the UI phase byte
 *           to 6 and re-arms phaseTimer with 0x3C.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Every read and store of the work-block index goes through the global array
 * itself: the original materialises the cell per use (lui/addiu in the two
 * counted arms, a folded lui plus lb in the confirm test), so a pointer local
 * that stays live across the calls scored 133/157 instructions and 616 bytes
 * with its first difference on the lui of the pointer.
 */
void drawShopSlotListThenStepSlotCursorAndAcceptOrCancel(void) {
  u8 count;
  s32 direction;
  s32 offset;
  s32 record_offset;

  count = func_801BDB7C(0);
  buildSlotList(0);
  func_8014F800(0x1C, 0x15, 0, 0xFF, func_801502D0(0x63));
  drawShopSlotPriceList(D_80148656[0], 0x98, 0x3F, 0, 0xFF);
  emitFlatLineRectangleBorder(0x11, ((s8)D_80148656[0] * 54 + 0x3E) & 0xFFFE,
                              0x80, 0x34, 1, 6);
  direction = func_8014E640(D_80145AA8 & 0x5000);

  if (direction & 0x4000) {
    func_8015DF18(0x100);
    if ((s8)(++D_80148656[0]) >= (s8)count) {
      D_80148656[0] = 0;
    }
  }
  if (direction & 0x1000) {
    func_8015DF18(0x100);
    if ((s8)(--D_80148656[0]) < 0) {
      D_80148656[0] = count - 1;
    }
  }

  if (D_80145AA8 & D_80145AC2) {
    offset = (s8)D_80148656[0] * 0x140;
    record_offset = D_80145FCC[offset] * 164;
    if (D_80144981[record_offset] != 0) {
      func_8015DF18(0x107);
      D_8014865B = 1;
      D_8014865F = 2;
      phaseTimer = 0x3C;
      D_80148651 += 6;
    } else {
      func_8015DF18(0x104);
      D_8014865D = 0;
      D_80148651 += 1;
    }
  }
  if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    D_8014865B = 0xF;
    D_80148651 = 6;
    phaseTimer = 0x3C;
  }
}
