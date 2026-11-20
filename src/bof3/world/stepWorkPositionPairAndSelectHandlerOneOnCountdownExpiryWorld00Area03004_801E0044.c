#include "bof3/world/area03004_internal.h"

/* @source 0x801E0044
 * @behavior Add work motion words 0x0C and 0x10 to position words 0x34 and 0x38, then
 * decrement byte 9. At zero, mask only the low halfwords of those positions
 * with 0x8000, set handler byte 1, and clear both motion words and mode byte
 * 2. Always step the pair sequence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairAndSelectHandlerOneOnCountdownExpiryWorld00Area03004(void) {
  u8* motionWork;
  u8* completionWork;
  u8* dispatchWork;
  s32 x;
  s32 y;
  s32 dx;
  s32 dy;
  u8 countdown;

  motionWork = D_1F800044;
  x = *(s32*)(motionWork + 0x34);
  dx = *(s32*)(motionWork + 0x0C);
  countdown = *(u8*)(motionWork + 9);
  *(s32*)(motionWork + 0x34) = x + dx;
  y = *(s32*)(motionWork + 0x38);
  countdown = (u8)(countdown - 1);
  *(u8*)(motionWork + 9) = countdown;
  dy = *(s32*)(motionWork + 0x10);
  completionWork = D_1F800044;
  *(s32*)(motionWork + 0x38) = y + dy;
  if (*(u8*)(completionWork + 9) == 0) {
    u16 xLowHalf;
    do {
      xLowHalf = *(u16*)(completionWork + 0x34);
    } while (0);
    *(u8*)(completionWork + 1) = 1;
    dispatchWork = D_1F800044;
    do {
      xLowHalf &= 0x8000;
    } while (0);
    *(u16*)(completionWork + 0x34) = xLowHalf;
    *(u32*)(completionWork + 0x0C) = 0;
    *(u32*)(completionWork + 0x10) = 0;
    *(u16*)(completionWork + 0x38) = *(u16*)(completionWork + 0x38) & 0x8000;
    *(u8*)(dispatchWork + 2) = 0;
  }
  func_8014D978();
}
