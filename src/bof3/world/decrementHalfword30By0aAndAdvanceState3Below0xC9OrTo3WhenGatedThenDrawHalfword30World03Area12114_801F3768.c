#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

void drawSpriteFrameAndMainRamTexture0World03Area12114_801F40DC(
    s16 arg0, s16 arg1);

/* @source 0x801F3768
 * @behavior Byte-1 state handler 7 of the byte-1 handler table 0x801F67D8 for
 * the work record published at the scratchpad pointer cell 0x1F800044.
 * Subtracts 0x0A from the signed halfword at record offset 0x30 and stores the
 * truncated halfword, then increments state byte 0x03 once that stored value
 * falls below 0xC9. When the shared byte 0x801454F2 is nonzero and work byte
 * 0x0B is nonzero, the byte at 0x80143BB0 is 2, or bit 0x100 of halfword
 * 0x80146258 is set, it then writes 3 to state byte 0x03 through a second read
 * of the pointer cell. Finally it passes mode 0x5C and the signed work halfword
 * 0x30 to the overlay emitter at 0x801F40DC. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By0aAndAdvanceState3Below0xC9OrTo3WhenGatedThenDrawHalfword30World03Area12114_801F3768(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = area12114WorkCursor_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0x0A);
  if (*half < 0xC9) {
    record[3]++;
  }
  if ((D_801454F2 != 0 && area12114WorkCursor_1F800044[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    area12114WorkCursor_1F800044[3] = 3;
  }
  emitter = area12114WorkCursor_1F800044;
  drawSpriteFrameAndMainRamTexture0World03Area12114_801F40DC(
      0x5C, FIELD_REF(s16, emitter, 0x30));
}
