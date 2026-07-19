#include "internal.h"

/* @behavior runs the secondary SCENA16 controller rooted at state 2.
 * @source 0x801F7230
 */
void func_801F7230(void) {
  volatile u8* object;
  u8           object_index;

  switch (SCENA16_D_80146875) {
    case 0:
      if (SCENA16_D_80143C40 == 0u) {
        func_8014ECAC(1u);
        func_8016C0C0(0, 0);
        func_80161C20(6u, 0x50, 8);
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
        SCENA16_D_80146258 |= 0x80u;
      }
      break;

    case 1:
      func_8014F800(0x7a, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)SCENA16_D_80010020);
      if (SCENA16_D_80143C40 == 0u) {
        SCENA16_D_80146876 = 0u;
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 2:
      SCENA16_D_80146876 = (u16)(SCENA16_D_80146876 + 1u);
      func_8016C0C0((s32)(s16)SCENA16_D_80146876, (s32)(s16)SCENA16_D_80146876);
      func_8014F800(0x7a, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)SCENA16_D_80010020);
      if (SCENA16_D_80146876 == 0x7fu) {
        func_8014ECAC(0u);
        SCENA16_D_80146876 = 0u;
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 3:
      if (SCENA16_D_80143C40 != 0u) {
        func_8014F800(0x7a, 100, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)SCENA16_D_80010020);
        break;
      }

      SCENA16_D_8014832E = 0x1fu;
      func_8015C088();
      func_8015C100();
      func_8014ECAC(1u);
      SCENA16_D_80146867 = 1u;
      SCENA16_D_80146876 = 0u;
      SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      break;

    case 4:
      if (SCENA16_D_80146867 == 0x54u) {
        func_8014ECAC(0u);
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 5:
      if (SCENA16_D_80143C40 == 0u) {
        SCENA16_D_8014832E = 0u;
        func_8014ECAC(1u);
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 6:
      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)SCENA16_D_80010022);
      if (SCENA16_D_80143C40 == 0u) {
        SCENA16_D_80146876 = 0x7fu;
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 7:
      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)SCENA16_D_80010022);
      SCENA16_D_80146876 = (u16)(SCENA16_D_80146876 - 1u);
      if (SCENA16_D_80146876 == 0u) {
        func_8014ECAC(0u);
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 8:
      if (SCENA16_D_80143C40 == 0u) {
        func_8019FA28(4u, 0x440000u, 0x80000u, 5u);
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      } else {
        func_8014F800(0x50, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)SCENA16_D_80010022);
      }
      break;

    case 9:
      if (SCENA16_D_80143F03 == 2u) {
        SCENA16_D_8014832E = 0x1fu;
        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 10:
      if (SCENA16_D_80146866 == 0x23u) {
        object_index = func_8019601C();
        SPAD_REF(volatile u8, 0x0u) = object_index;

        if (object_index != 0xffu) {
          object =
              PSX_PTR(volatile u8, 0x80143fc8u) + ((u32)object_index * 0x74u);
          object[0] = 1u;
          object[5] = 0x13u;
          *(volatile s32*)(object + 0x64) =
              (s32)((s16)SCENA16_D_801492D8 + 0x180);
          *(volatile s32*)(object + 0x68) = (s32)(s16)SCENA16_D_801492DA;
          *(volatile s32*)(object + 0x6c) = (s32)(s16)SCENA16_D_801492DC;
          object[9] = 0x60u;
        }

        SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
      }
      break;

    case 11:
      if (SCENA16_D_80146867 == 0x60u) {
        func_8019FA28(0x1fu, 0x70000u, 0x140000u, 0u);
        SCENA16_D_80143F1D = 0xffu;
        func_80161CD0(6u, 0x50, 0x40);
        SCENA16_D_80146866 = 0u;
        SCENA16_D_80146874 = 3;
        SCENA16_D_80146875 = 0u;
      }
      break;
  }
}
