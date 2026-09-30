#include "bof3/ui/game00_internal.h"

/* @behavior walks the 8-byte-stride bounding-box records reached from the
 * pointer table D_80181164[index] and returns a pointer to the first record
 * whose box contains the point (x, y): record[0] <= x, record[1] <= y,
 * record[2] >= x and record[3] >= y. The scan advances both the record pointer
 * and its box-end pointer eight bytes at a time with no count bound, so the
 * box list is expected to contain the point.
 * @source 0x801A0490
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* func_801A0490(u8 x, u8 y, u16 index) {
  u8* record;
  u8* bounds;
  s32 px;
  s32 py;

  record = D_80181164[index];
  bounds = record + 3;
  px = x;
  py = y;
  for (;;) {
    if (px < record[0]) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (py < bounds[-2]) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (bounds[-1] < px) {
      bounds += 8;
      record += 8;
      continue;
    }
    if (bounds[0] < py) {
      bounds += 8;
      record += 8;
      continue;
    }
    return record;
  }
}
