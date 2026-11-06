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

/* @source 0x801F3940
 * @behavior Byte-1 state handler 9 of the byte-1 handler table 0x801F67D8 for
 * the work record published at the scratchpad pointer cell 0x1F800044. Adds
 * 0x0A to work halfword 0x30 and compares the stored signed halfword against
 * 0xF0, clearing state byte 0x03 once the height reaches that value. Unless
 * the shared mode byte 0x801454F2 is set, the byte at 0x80143BB0 is 2, or bit
 * 0x100 of halfword 0x80146258 is set, it then arms state byte 0x03 at 1
 * through a second read of the pointer cell. Finally it passes mode 0x5C and
 * the signed work halfword 0x30 to the overlay emitter at 0x801F40DC. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By0aAndSetState3To0AtF0OrTo1UnlessGatedThenDrawHalfword30World03Area12114_801F3940(void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = area12114WorkCursor_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x0A);
  if (*half >= 0xF0) {
    record[3] = 0;
  }
  if (D_801454F2 == 0 && D_80143BB0 != 2 && (D_80146258 & 0x100u) == 0) {
    area12114WorkCursor_1F800044[3] = 1;
  }
  emitter = area12114WorkCursor_1F800044;
  func_801F40DC(0x5C, FIELD_REF(s16, emitter, 0x30));
}
