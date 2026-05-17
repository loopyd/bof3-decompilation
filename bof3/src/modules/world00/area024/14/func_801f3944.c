#include "internal.h"

/* does: projects one origin point plus two rotated/scaled local offsets from
 * the packed work record, then emits a translucent G3 with one grayscale end
 * faded against two black vertices.
 * @source: 0x801f3944 FUN_801f3944
 */
void func_801f3944(const void* arg0) {
  const u8* work;
  u32       primitive;
  s32       point[3];
  s16       angle;
  s16       scale;
  s16       color;
  u8        color_byte;

  work = (const u8*)arg0;
  primitive = (u32)WORLD00_AREA024_PRIMITIVE_PTR;

  func_8017a97c((void*)primitive);
  func_8017a904((void*)primitive, 1);
  func_801aff04(arg0, (void*)(primitive + 8));

  angle = *(const s16*)(work + 0x24);
  scale = *(const s16*)(work + 0x28);

  point[0] = *(const s32*)(work + 0) +
             (((((s32)rcos(angle) * *(const s16*)(work + 0x10)) -
                ((s32)rsin(angle) * *(const s16*)(work + 0x12))) >>
               8) *
              scale);
  point[1] = *(const s32*)(work + 4) +
             (((((s32)rsin(angle) * *(const s16*)(work + 0x10)) +
                ((s32)rcos(angle) * *(const s16*)(work + 0x12))) >>
               8) *
              scale);
  point[2] = *(const s32*)(work + 8) +
             (((s32) * (const s16*)(work + 0x14) << 12) * scale);
  func_801aff04(point, (void*)(primitive + 0x10));

  point[0] = *(const s32*)(work + 0) +
             (((((s32)rcos(angle) * *(const s16*)(work + 0x18)) -
                ((s32)rsin(angle) * *(const s16*)(work + 0x1a))) >>
               8) *
              scale);
  point[1] = *(const s32*)(work + 4) +
             (((((s32)rsin(angle) * *(const s16*)(work + 0x18)) +
                ((s32)rcos(angle) * *(const s16*)(work + 0x1a))) >>
               8) *
              scale);
  point[2] = *(const s32*)(work + 8) +
             (((s32) * (const s16*)(work + 0x1c) << 12) * scale);
  func_801aff04(point, (void*)(primitive + 0x18));

  color = *(const s16*)(work + 0x2a);
  if (color < 0) {
    color_byte = 0u;
  } else if (color < 0x100) {
    color_byte = (u8)color;
  } else {
    color_byte = 0xffu;
  }

  *(volatile u8*)(primitive + 4) = color_byte;
  *(volatile u8*)(primitive + 5) = color_byte;
  *(volatile u8*)(primitive + 6) = color_byte >> 1;
  *(volatile u8*)(primitive + 0xc) = 0u;
  *(volatile u8*)(primitive + 0xd) = 0u;
  *(volatile u8*)(primitive + 0xe) = 0u;
  *(volatile u8*)(primitive + 0x14) = 0u;
  *(volatile u8*)(primitive + 0x15) = 0u;
  *(volatile u8*)(primitive + 0x16) = 0u;
  func_8014e5a0(1u, 0x1cu);
}
