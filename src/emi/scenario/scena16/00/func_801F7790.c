#include "internal.h"

/* @behavior runs the secondary SCENA16 controller rooted at state 3.
 * @source 0x801F7790
 */
void func_801F7790(void) {
  volatile u16* timer;

  switch (D_80146875) {
    case 0:
      if (D_80143C40 == 0u) {
        func_8014ECAC(1u);
        goto increment_state;
      }
      break;

    case 1:
      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010004);
      if (D_80143C40 == 0u) {
        D_80146876 = 0x7fu;
        goto increment_state;
      }
      break;

    case 2:
      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010004);
      D_80146876 = (u16)(D_80146876 - 1u);
      if (D_80146876 == 0u) {
        func_8014ECAC(0u);
        goto increment_state;
      }
      break;

    case 3:
      if (D_80143C40 != 0u) {
        func_8014F800(0x50, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010004);
        break;
      }

      func_8014ECAC(1u);
      D_80146866 = 0x30u;
      D_8014832E = 0x1fu;
      SPAD_REF(volatile u8, 0x0u) = func_8019601C();

      if (SPAD_REF(volatile u8, 0x0u) != 0xffu) {
        volatile u8* object;
        u32          object_index;

        object_index = (u32)SPAD_REF(volatile u8, 0x0u);
        object = PSX_PTR(volatile u8, 0x80143fc8u) + (object_index * 0x74u);
        object[0] = 1u;
        object[5] = 0x13u;
        *(volatile s32*)(object + 0x64) =
            (s32)((s16)D_801492D8 + 0x1c0);
        *(volatile s32*)(object + 0x68) = (s32)(s16)D_801492DA;
        *(volatile s32*)(object + 0x6c) = (s32)(s16)D_801492DC;
        object[9] = 0xffu;
      }

      goto increment_state;

    case 4:
      if (D_80143C40 == 0u) {
        D_80146876 = 0u;
        goto increment_state;
      }
      break;

    case 5:
      func_8014F800(0x48, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010006);
      timer = &D_80146876;
      *timer = (u16)(*timer + 1u);
      if (*timer != 0x20u) {
        func_801F83B0((u32)((u8)*timer));
        break;
      }
      copyPaletteBlock();
      *timer = 0x7fu;
      goto increment_state;

    case 6:
      func_8014F800(0x48, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010006);
      D_80146876 = (u16)(D_80146876 - 1u);
      if (D_80146876 == 0u) {
        D_80146876 = 0x20u;
        goto increment_state;
      }
      break;

    case 7:
      timer = &D_80146876;
      *timer = (u16)(*timer - 1u);
      if (*timer != 0u) {
        func_8014F800(0x48, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010006);
        func_801F83B0((u32)(*(volatile u8*)timer));
        break;
      }
      copyPaletteBlock();
      *timer = 0x1eu;
      goto increment_state;

    case 8:
      D_80146876 = (u16)(D_80146876 - 1u);
      if (D_80146876 == 0u) {
        goto increment_state;
      }
      break;

    case 9:
      func_80161C20(2u, 100, 0x20);
      func_8019FA28(2u, 0x340000u, 0x430000u, 3u);
      D_80146874 = 4;
      D_80146875 = 0u;
      break;
  }

footer:
  if (D_80149314 == 0x4400u) {
    D_80149314 = 0x6200u;
    D_80149322 = (u16)(D_80149322 - 0x1eu);
    D_8014930C += -0x1e0000;
    D_80147A90 += -0x1e0000;
    D_80143F80 += -0x1e0000;
    func_80154698();
  }

  return;

increment_state:
  D_80146875 = (u8)(D_80146875 + 1u);
  goto footer;
}
