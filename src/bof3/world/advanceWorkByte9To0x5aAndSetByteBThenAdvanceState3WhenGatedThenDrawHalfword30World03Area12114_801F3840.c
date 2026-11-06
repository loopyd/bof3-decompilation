#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;

void func_801F40DC(s16 arg0, s16 arg1);

/* @source 0x801F3840
 * @behavior Byte-1 state handler 8 of the byte-1 handler table 0x801F67D8 for
 * the work record published at the scratchpad pointer cell 0x1F800044. While
 * work byte 0x0B is still zero it increments work byte 0x09 and, once that
 * stored byte reaches 0x5A, increments work byte 0x0B as well. When the shared
 * byte 0x801454F2 is nonzero and work byte 0x0B is nonzero, the byte at
 * 0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set, it then
 * increments state byte 0x03 through a second read of the pointer cell.
 * Finally it passes mode 0x5C and the signed work halfword 0x30 to the overlay
 * emitter at 0x801F40DC. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9To0x5aAndSetByteBThenAdvanceState3WhenGatedThenDrawHalfword30World03Area12114_801F3840(
    void) {
  u8* record;
  u8* emitter;

  record = area12114WorkCursor_1F800044;
  if (record[0x0B] == 0) {
    record[9]++;
    record = area12114WorkCursor_1F800044;
    if (record[9] >= 0x5A) {
      record[0x0B]++;
    }
  }
  if ((D_801454F2 != 0 && area12114WorkCursor_1F800044[0x0B] != 0) ||
      D_80143BB0 == 2 || (D_80146258 & 0x100u) != 0) {
    area12114WorkCursor_1F800044[3]++;
  }
  emitter = area12114WorkCursor_1F800044;
  func_801F40DC(0x5C, FIELD_REF(s16, emitter, 0x30));
}
