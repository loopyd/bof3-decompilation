#include "internal.h"

/* does: shifts the 32-entry projected trail by one slot, then recomputes the
 * current projected head from the base position, scale, and angle.
 * @source: 0x801f2e3c FUN_801f2e3c
 */
void func_801f2e3c(void* arg0) {
  u8* work;
  u8  i;
  s32 point[3];
  u8  scratch[0x20];

  work = (u8*)arg0;
  i = 0x1fu;

  do {
    *(u32*)(work + ((u32)i * 4u) + 0x18u) =
        *(u32*)(work + ((u32)i * 4u) + 0x14u);
    i -= 1u;
  } while (i != 0u);

  point[0] =
      *(s32*)(work + 0x00) +
      ((((s32) * (s32*)(work + 0x10) * rcos(*(s16*)(work + 0x14))) >> 12));
  point[1] =
      *(s32*)(work + 0x04) +
      ((((s32) * (s32*)(work + 0x10) * rsin(*(s16*)(work + 0x14))) >> 12));
  point[2] = *(s32*)(work + 0x08) << 8;

  func_801afe18(scratch);
  func_801aff04(point, work + 0x18);
}
