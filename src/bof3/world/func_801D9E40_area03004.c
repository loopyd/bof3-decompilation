#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801D9E40
 * @behavior Dispatches the shared pad-state mask D_80145AA8 & 0xD4 over the
 * AREA030 mode handlers. The 0x04, 0x40 and 0x80 keys set modeByte to 7 and
 * D_8014403D to 4 before arming the work record published at the scratchpad
 * cursor 0x1F800044: byte 2 holds the per-key mode constant (0x0A for 0x04,
 * 9 for 0x80, and 9 or 2 for the two 0x40 outcomes) and byte 3 is cleared.
 * The 0x10 key only advances the record's byte 3. The 0x40 key requires the
 * D_80145026 / D_80145028 level flags; when func_801E1C10 reports a full icon
 * row it arms byte 3 with the mode constant and sets D_801E3204, otherwise it
 * clears D_801E3204 and arms byte 2 with 2.
 * @status partial
 * @match 82.57
 * @residual scheduler emits the three mode constants inside the case bodies instead of the dispatch-branch delay slots
 */
void func_801D9E40(void) {
  u8 *work;

  switch (D_80145AA8 & 0xD4) {
  case 0x10:
    modeByte = 7;
    D_8014403D = 4;
    work = D_1F800044;
    work[3]++;
    break;
  case 0x80:
    modeByte = 7;
    D_8014403D = 4;
    work = D_1F800044;
    work[2] = 9;
    D_1F800044[3] = 0;
    break;
  case 0x40:
    modeByte = 7;
    if (D_80145026 == 0 || D_80145028 == 0) {
      work = D_1F800044;
      D_8014403D = 4;
      work[2] = 9;
      D_1F800044[3] = 0;
    } else {
      if ((u8)func_801E1C10() != 0) {
        if (D_801E3204 == 0) {
          D_801E3204 = 1;
          work = D_1F800044;
          D_8014403D = 4;
          work[3] = 7;
          break;
        }
      } else {
        D_801E3204 = 0;
      }
      work = D_1F800044;
      work[2] = 2;
      D_1F800044[3] = 0;
    }
    break;
  case 0x04:
    modeByte = 7;
    D_8014403D = 4;
    work = D_1F800044;
    work[2] = 0xA;
    D_1F800044[3] = 0;
    break;
  }
}
