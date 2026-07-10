#include "internal.h"

/* does: recomputes the projected trail head from the current base position,
 * then fills the remaining 31 slots with that same head position.
 * @source: 0x801f304c FUN_801f304c
 */
void func_801f304c(void* arg0) {
  u8* work;
  u8  i;
  s32 point[3];
  u8  scratch[0x20];

  work = (u8*)arg0;

  point[0] =
      *(s32*)(work + 0x00) +
      ((((s32) * (s32*)(work + 0x10) * rcos(*(s16*)(work + 0x14))) >> 12));
  point[1] =
      *(s32*)(work + 0x04) +
      ((((s32) * (s32*)(work + 0x10) * rsin(*(s16*)(work + 0x14))) >> 12));
  point[2] = *(s32*)(work + 0x08) << 8;

  func_801afe18(scratch);
  func_801aff04(point, work + 0x18);

  i = 1u;
  do {
    *(u32*)(work + ((u32)i * 4u) + 0x18u) = *(u32*)(work + 0x18u);
    i += 1u;
  } while (i < 0x20u);
}
