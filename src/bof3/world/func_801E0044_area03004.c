#include "bof3/world/area03004_internal.h"

/* @source 0x801E0044
 * @behavior AREA030 work-record movement integrator: adds the step words at
 * offsets 0x0C/0x10 into the 32-bit position words at 0x34/0x38 of the scratch
 * work record published at the scratchpad cursor 0x1F800044 and counts the
 * record byte 0x09 down; when that count reaches zero it collapses both
 * positions to their sign bit, arms record byte 0x01, clears both step words
 * and record byte 0x02, then always runs the shared helper func_8014D978.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E0044(void) {
  u8* work;
  u8* next;
  u8* armed;
  s32 x;
  s32 y;
  s32 dx;
  s32 dy;
  u8 count;

  work = D_1F800044;
  x = *(s32*)(work + 0x34);
  dx = *(s32*)(work + 0x0C);
  count = *(u8*)(work + 9);
  *(s32*)(work + 0x34) = x + dx;
  y = *(s32*)(work + 0x38);
  count = (u8)(count - 1);
  *(u8*)(work + 9) = count;
  dy = *(s32*)(work + 0x10);
  next = D_1F800044;
  *(s32*)(work + 0x38) = y + dy;
  if (*(u8*)(next + 9) == 0) {
    u16 xSign;
    do {
      xSign = *(u16*)(next + 0x34);
    } while (0);
    *(u8*)(next + 1) = 1;
    armed = D_1F800044;
    do {
      xSign &= 0x8000;
    } while (0);
    *(u16*)(next + 0x34) = xSign;
    *(u32*)(next + 0x0C) = 0;
    *(u32*)(next + 0x10) = 0;
    *(u16*)(next + 0x38) = *(u16*)(next + 0x38) & 0x8000;
    *(u8*)(armed + 2) = 0;
  }
  func_8014D978();
}
