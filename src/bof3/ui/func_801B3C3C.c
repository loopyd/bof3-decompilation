#include "bof3/ui/game00_internal.h"

/* @behavior Finds the first eight-byte bounding-box record in the list reached
 * from the pointer table D_80181164[index], where index is the pending world id
 * D_80143F00, and returns a pointer to the record whose box contains the point
 * (x, y): record[0] <= x, record[1] <= y, record[2] >= x and record[3] >= y.
 * The scan advances both the record pointer and its box-end pointer eight bytes
 * at a time with no count bound, so the box list is expected to contain the
 * point.
 * @source 0x801B3C3C
 * @status exact
 * @match 100.00
 * @residual none
 * The four box tests share one labelled advance block. The original reuses the
 * shared block's entry instruction as the fourth test's delay slot content and
 * retargets that branch past it, which only happens when the advance block is a
 * real label instead of four duplicated `continue` arms.
 */
u8* func_801B3C3C(u8 x, u8 y) {
  u8* record;
  u8* bounds;
  s32 px;
  s32 py;

  record = D_80181164[D_80143F00];
  bounds = record + 3;
  px = x;
  py = y;
loop:
  if (px < record[0]) {
    goto advance;
  }
  if (py < bounds[-2]) {
    goto advance;
  }
  if (bounds[-1] < px) {
    goto advance;
  }
  if (bounds[0] < py) {
    goto advance;
  }
  return record;
advance:
  bounds += 8;
  record += 8;
  goto loop;
}
