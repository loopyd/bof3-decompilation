#include "internal.h"

/* does: projects one local sprite work entry, centers a translucent FT4 on the
 * projected point using the half-size derived from `+0x24`, then queues the
 * matching local effect.
 * @source: 0x801f2df8 FUN_801f2df8
 */
void func_801f2df8(const void* arg0) {
  s16                             screen[2];
  u16                             size[2];
  const World00Area024SpriteWork* work;
  const void*                     object;
  u32                             primitive;
  u8                              color;

  work = (const World00Area024SpriteWork*)arg0;
  object = (const void*)((const u8*)arg0 + 4u);
  primitive = (u32)WORLD00_AREA024_PRIMITIVE_PTR;

  func_8017a9b8((void*)primitive);
  func_8017a904((void*)primitive, 1);

  func_801aff04(object, screen);
  size[0] = *(const volatile u16*)&work->field_24;
  size[1] = *(const volatile u16*)&work->field_24;
  func_801affd8(object, size, size);

  *(volatile s16*)(primitive + 8) = screen[0] - ((s16)size[0] >> 1);
  *(volatile s16*)(primitive + 10) = screen[1] - ((s16)size[1] >> 1);
  *(volatile s16*)(primitive + 0x10) =
      (screen[0] - ((s16)size[0] >> 1)) + (s16)size[0];
  *(volatile s16*)(primitive + 0x12) = screen[1] - ((s16)size[1] >> 1);
  *(volatile s16*)(primitive + 0x18) = screen[0] - ((s16)size[0] >> 1);
  *(volatile s16*)(primitive + 0x1a) =
      (screen[1] - ((s16)size[1] >> 1)) + (s16)size[1];
  *(volatile s16*)(primitive + 0x20) =
      (screen[0] - ((s16)size[0] >> 1)) + (s16)size[0];
  *(volatile u8*)(primitive + 0xd) = 0x30u;
  *(volatile u8*)(primitive + 0x15) = 0x30u;
  *(volatile u8*)(primitive + 0xc) = 0xe0u;
  *(volatile u8*)(primitive + 0x14) = 0xffu;
  *(volatile u8*)(primitive + 0x1c) = 0xe0u;
  *(volatile u8*)(primitive + 0x1d) = 0x4fu;
  *(volatile u8*)(primitive + 0x24) = 0xffu;
  *(volatile u8*)(primitive + 0x25) = 0x4fu;
  *(volatile s16*)(primitive + 0x22) =
      (screen[1] - ((s16)size[1] >> 1)) + (s16)size[1];
  *(volatile u16*)(primitive + 0xe) = func_8017a6f0(0xa0, 0x1e3);
  *(volatile u16*)(primitive + 0x16) = func_8017a620(0, 1, 0x2c0, 0x100);
  color = work->field_03;
  *(volatile u8*)(primitive + 4) = color;
  color = work->field_03;
  *(volatile u8*)(primitive + 5) = color;
  color = work->field_03;
  *(volatile u8*)(primitive + 6) = color;

  func_80155a08(work->field_04, work->field_08, 2, 0x28);
}
