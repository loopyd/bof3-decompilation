#include "bof3/world/area03004_internal.h"

/* @source 0x801E0680
 * @behavior Under the three shared gates, return success when work byte 6 is 0x16 and
 * the shared low nibble matches the mode, or when matchesProjectedWorkBounds
 * accepts the original x, y, signed-counter and byte-range arguments. On every
 * failure path, set work handler byte 1 to 1 and return zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 checkWorkEntryGateOrSelectHandlerOneWorld00Area03004(void) {
  Area030WorkRecord* workRecord;
  u8* sharedByteAddress;
  u8* sharedEntryBase;
  u8* sharedBytes;
  u8 sharedByteValue;
  s32 referenceByte;

  if (D_80143FC9 == 4 && D_801E3208 == 0xFF && D_80145E92 == 3) {
    if (D_1F800044[6] == 0x16 && (D_801E3210[0x0E] & 0xF) == D_80143FC9) {
      return 1;
    }
    sharedBytes = D_801E3210;
    sharedByteAddress = (u8*)&D_80143FCF;
    sharedByteValue = *sharedByteAddress;
    /* The sharedBytes byte enters the range difference as a promoted word value. */
    referenceByte = sharedBytes[0x0F];
    sharedEntryBase = sharedByteAddress - 7;
    workRecord = (Area030WorkRecord*)D_1F800044;
    if (matchesProjectedWorkBounds(workRecord->x_34, workRecord->y_38, workRecord->counter_3E,
                      (u8)(sharedByteValue - referenceByte + 3),
                      sharedEntryBase) != 0) {
      return 1;
    }
  }
  D_1F800044[1] = 1;
  return 0;
}
