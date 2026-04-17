#include "internal.h"

/* does: projects the current center point and emits a 32-step white G3 fan,
 * storing each projected outer point into the shared ring tables.
 * @source: 0x801f318c FUN_801f318c
 */
void func_801f318c(s16 arg0) {
  s32 point[3];
  s16 center[2];
  s16 outer[2];
  s16 angle;
  u8  i;
  u32 primitive;
  u8  scratch[0x20];

  func_8017c2d8((void*)BOF3_WORLD00_AREA028_PRIMITIVE_PTR, 0, 1,
                func_8017a620(0, 2, 0x3c0, 0), 0);
  func_8014e5a0(3u, 0x0cu);
  func_801afe18(scratch);

  point[0] = *(volatile s32*)(0x1f800044u + 0x34u);
  point[1] = *(volatile s32*)(0x1f800044u + 0x38u);
  point[2] = *(volatile s32*)(0x1f800044u + 0x3cu);
  func_801aff04(point, center);
  BOF3_WORLD00_AREA028_CENTER_X = (u16)center[0];
  BOF3_WORLD00_AREA028_CENTER_Y = (u16)center[1];

  angle = -0x80;
  point[0] =
      *(volatile s32*)(0x1f800044u + 0x34u) + (((s32)arg0 * rcos(angle)) >> 4);
  point[1] =
      *(volatile s32*)(0x1f800044u + 0x38u) + (((s32)arg0 * rsin(angle)) >> 4);
  point[2] = *(volatile s32*)(0x1f800044u + 0x3cu);
  func_801aff04(point, outer);

  i = 0u;
  do {
    primitive = (u32)BOF3_WORLD00_AREA028_PRIMITIVE_PTR;
    func_8017a97c((void*)primitive);
    func_8017a904((void*)primitive, 1);

    *(volatile s16*)(primitive + 8) = center[0];
    *(volatile s16*)(primitive + 10) = center[1];
    *(volatile s16*)(primitive + 0x10) = outer[0];
    *(volatile s16*)(primitive + 0x12) = outer[1];

    angle = (s16)(angle + 0x80);
    point[0] = *(volatile s32*)(0x1f800044u + 0x34u) +
               (((s32)arg0 * rcos(angle)) >> 4);
    point[1] = *(volatile s32*)(0x1f800044u + 0x38u) +
               (((s32)arg0 * rsin(angle)) >> 4);
    point[2] = *(volatile s32*)(0x1f800044u + 0x3cu);
    func_801aff04(point, outer);

    *(volatile s16*)(primitive + 0x18) = outer[0];
    *(volatile s16*)(primitive + 0x1a) = outer[1];
    BOF3_WORLD00_AREA028_RING_X(i) = (u16)outer[0];
    BOF3_WORLD00_AREA028_RING_Y(i) = (u16)outer[1];
    *(volatile u8*)(primitive + 4) = 0xffu;
    *(volatile u8*)(primitive + 5) = 0xffu;
    *(volatile u8*)(primitive + 6) = 0xffu;
    *(volatile u8*)(primitive + 0xc) = 0u;
    *(volatile u8*)(primitive + 0xd) = 0u;
    *(volatile u8*)(primitive + 0xe) = 0u;
    *(volatile u8*)(primitive + 0x14) = 0u;
    *(volatile u8*)(primitive + 0x15) = 0u;
    *(volatile u8*)(primitive + 0x16) = 0u;
    func_8014e5a0(3u, 0x1cu);

    i += 1u;
  } while (i < 0x20u);
}
