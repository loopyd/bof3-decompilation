#include "internal.h"

/* @behavior updates the local ring-center point from the scaled work vector,
 * rejects the draw if that center falls outside the 32-point ring, and
 * otherwise emits one TILE_1 marker tinted from `field_08`.
 * @source 0x801f2d3c FUN_801f2d3c
 */
void func_801f2d3c(void) {
  World00Area028Work* work;
  u8                  i;
  u8                  next_i;
  s16                 dx0;
  s16                 dy0;
  s16                 dx1;
  s16                 dy1;
  s32                 shade;
  u8                  color;
  u32                 primitive;

  work = WORLD00_AREA028_WORK_PTR;

  work->field_0c = (s16)(WORLD00_AREA028_CENTER_X +
                         ((work->field_04 << 7) / work->field_08));
  work->field_0e = (s16)(WORLD00_AREA028_CENTER_Y +
                         ((work->field_06 << 7) / work->field_08));

  i = 0u;
  next_i = 1u;
  do {
    dx0 = (s16)(WORLD00_AREA028_RING_X(next_i & 0x1fu) -
                WORLD00_AREA028_RING_X(i));
    dy0 = (s16)(WORLD00_AREA028_RING_Y(next_i & 0x1fu) -
                WORLD00_AREA028_RING_Y(i));
    dx1 = (s16)(work->field_0c - WORLD00_AREA028_RING_X(i));
    dy1 = (s16)(work->field_0e - WORLD00_AREA028_RING_Y(i));
    if (((s32)dx0 * (s32)dy1) - ((s32)dy0 * (s32)dx1) < 0) {
      return;
    }
    i = next_i;
    next_i += 1u;
  } while (i < 0x20u);

  primitive = (u32)WORLD00_AREA028_PRIMITIVE_PTR;
  func_8017aa30((void*)primitive);
  func_8017a904((void*)primitive, 0);
  *(volatile s16*)(primitive + 8) = work->field_0c;
  *(volatile s16*)(primitive + 10) = work->field_0e;

  shade = (work->field_08 - 0x80) * 0xc0;
  if (shade < 0) {
    shade += 0x1ff;
  }
  color = (u8)(-0x40 - (shade >> 9));

  *(volatile u8*)(primitive + 4) = color;
  *(volatile u8*)(primitive + 5) = color;
  *(volatile u8*)(primitive + 6) = color;
  func_8014e5a0(3u, 0x0cu);
}
