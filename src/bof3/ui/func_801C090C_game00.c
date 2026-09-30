#include "bof3/ui/game00_internal.h"

/* @behavior publishes the pair D_80145EC4/D_80145EC8 for the mode byte arg2:
 * with fewer than two D_80146254 records the two arguments are stored
 * unchanged, otherwise the byte offset (arg2 << 2) & 0x3F8 selects one
 * eight-byte record from D_801CD47C whose first word doubled is added to the
 * first argument and whose second word doubled is added to the second; when the
 * record flagged by D_80145F00, or a later 0x140-byte-stride record, is live,
 * that same record is added once more to both published words, and the mode byte
 * is stored into the record leading byte D_80145E98.
 * @source 0x801C090C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801C090C(u32 arg0, u32 arg1, u8 arg2) {
  u8 count = D_80146254;
  s32 i;

  if (count >= 2) {
    u32 x;
    u32 y;

    x = arg0 + FIELD_REF(s32, D_801CD47C, (arg2 << 2) & 0x3F8) * 2;
    FIELD_REF(u32, &D_80145EC4, 0) = x;
    y = arg1 + FIELD_REF(s32, D_801CD480, (arg2 << 2) & 0x3F8) * 2;
    FIELD_REF(u32, &D_80145EC4, 4) = y;
    if (D_80145F00[0] != 0) {
      FIELD_REF(u32, &D_80145EC4, 0) =
          x + FIELD_REF(s32, D_801CD47C, (arg2 << 2) & 0x3F8);
      FIELD_REF(u32, &D_80145EC4, 4) =
          y + FIELD_REF(s32, D_801CD480, (arg2 << 2) & 0x3F8);
    }
    D_80145E98[0].field_00 = arg2;
    for (i = 1; i < count; i++) {
      s32 offset = i * 0x140;

      if (D_80145F00[offset] != 0) {
        FIELD_REF(u32, &D_80145EC4, 0) +=
            FIELD_REF(s32, D_801CD47C, (arg2 << 2) & 0x3F8);
        FIELD_REF(u32, &D_80145EC4, 4) +=
            FIELD_REF(s32, D_801CD480, (arg2 << 2) & 0x3F8);
        break;
      }
    }
  } else {
    FIELD_REF(u32, &D_80145EC4, 0) = arg0;
    FIELD_REF(u32, &D_80145EC4, 4) = arg1;
    D_80145E98[0].field_00 = arg2;
  }
}
