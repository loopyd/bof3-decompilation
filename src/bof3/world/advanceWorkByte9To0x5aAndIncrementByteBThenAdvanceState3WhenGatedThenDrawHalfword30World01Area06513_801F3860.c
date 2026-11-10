#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

void drawSpriteFrameAndMainRamTexture2World01Area06513_801F40FC(s16 arg0,
                                                               s16 arg1);

/* @source 0x801F3860
 * @behavior Byte-2 state handler 6 of the eight-entry handler table based at
 * 0x801F5290, indexed by work-record byte 0x02 through the dispatcher at
 * 0x801F3500, over the work record published at scratchpad pointer cell
 * 0x1F800044. While record byte 0x0B is still zero it increments record byte
 * 0x09 and, once that stored byte reaches 0x5A, increments record byte 0x0B
 * through a second read of the pointer cell. It then increments state byte
 * 0x03 through another read of the cell when the shared byte 0x801454F2 is
 * nonzero and record byte 0x0B is nonzero, or the byte at 0x80143BB0 is 2, or
 * bit 0x100 of halfword 0x80146258 is set. Finally it reads the cell again and
 * passes 0x5C and the signed halfword at offset 0x30 to the local emitter at
 * 0x801F40FC. Takes no arguments and returns nothing. The 0x100 bytes are
 * byte-identical to the spans at 0x801F382C in emi/world03/area115/13 and
 * 0x801F3840 in emi/world03/area121/14 apart from the low byte of the internal
 * `jal` word that encodes each overlay's own emitter address, so this body
 * transcribes that family's statement order with this target's emitter.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9To0x5aAndIncrementByteBThenAdvanceState3WhenGatedThenDrawHalfword30World01Area06513_801F3860(
    void) {
  u8* record;
  u8* emitter;

  record = D_1F800044;
  if (record[0x0B] == 0) {
    record[9]++;
    record = D_1F800044;
    if (record[9] >= 0x5A) {
      record[0x0B]++;
    }
  }
  if ((D_801454F2 != 0 && D_1F800044[0x0B] != 0) || D_80143BB0 == 2 ||
      (D_80146258 & 0x100u) != 0) {
    D_1F800044[3]++;
  }
  emitter = D_1F800044;
  drawSpriteFrameAndMainRamTexture2World01Area06513_801F40FC(
      0x5C, FIELD_REF(s16, emitter, 0x30));
}
