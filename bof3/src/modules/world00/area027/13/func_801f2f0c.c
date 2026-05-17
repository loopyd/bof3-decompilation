#include "internal.h"

/* does: emits a 31-segment Gouraud line strip across the projected trail,
 * fading the leading endpoint from bright red toward black.
 * @source: 0x801f2f0c FUN_801f2f0c
 */
void func_801f2f0c(const void* arg0) {
  const u8* work;
  u8        i;
  u32       primitive;
  u16       red0;
  u16       red1;

  work = (const u8*)arg0;

  func_8017c2d8((void*)WORLD00_AREA027_PRIMITIVE_PTR, 0, 0,
                func_8017a620(0, 1, 0x380, 0x100), 0);
  func_8014e5a0(1u, 0x0cu);

  i = 0u;
  do {
    primitive = (u32)WORLD00_AREA027_PRIMITIVE_PTR;
    func_8017aa94((void*)primitive);
    func_8017a904((void*)primitive, 1);

    *(volatile s16*)(primitive + 8) =
        *(const s16*)(work + ((u32)i * 4u) + 0x18u);
    *(volatile s16*)(primitive + 10) =
        *(const s16*)(work + ((u32)i * 4u) + 0x1au);
    *(volatile s16*)(primitive + 0x10) =
        *(const s16*)(work + (((u32)i + 1u) * 4u) + 0x18u);
    *(volatile s16*)(primitive + 0x12) =
        *(const s16*)(work + (((u32)i + 1u) * 4u) + 0x1au);

    red0 = (0x20u - (u16)i) * 8u;
    if (red0 > 0xffu) {
      red0 = 0xffu;
    }
    red1 = (0x1fu - (u16)i) * 8u;
    if (red1 > 0xffu) {
      red1 = 0xffu;
    }

    *(volatile u8*)(primitive + 4) = (u8)red0;
    *(volatile u8*)(primitive + 5) = 0u;
    *(volatile u8*)(primitive + 6) = 0u;
    *(volatile u8*)(primitive + 0xc) = (u8)red1;
    *(volatile u8*)(primitive + 0xd) = 0u;
    *(volatile u8*)(primitive + 0xe) = 0u;
    func_8014e5a0(1u, 0x14u);

    i += 1u;
  } while (i < 0x1fu);
}
