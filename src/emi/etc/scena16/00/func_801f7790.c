#include "internal.h"

/* @behavior runs the secondary SCENA16 controller rooted at state 3.
 * @source 0x801f7790 FUN_801f7790
 */
void func_801f7790(void) {
  vu16* timer;

  switch (BOF3_SCENA16_DAT_80146875) {
    case 0:
      if (BOF3_SCENA16_DAT_80143c40 == 0u) {
        func_8014ecac(1u);
        goto increment_state;
      }
      break;

    case 1:
      func_8014f800(0x50, 0x50, 0, 0xffu,
                    0x80010000u + (u32)BOF3_SCENA16_DAT_80010004);
      if (BOF3_SCENA16_DAT_80143c40 == 0u) {
        BOF3_SCENA16_DAT_80146876 = 0x7fu;
        goto increment_state;
      }
      break;

    case 2:
      func_8014f800(0x50, 0x50, 0, 0xffu,
                    0x80010000u + (u32)BOF3_SCENA16_DAT_80010004);
      BOF3_SCENA16_DAT_80146876 = (u16)(BOF3_SCENA16_DAT_80146876 - 1u);
      if (BOF3_SCENA16_DAT_80146876 == 0u) {
        func_8014ecac(0u);
        goto increment_state;
      }
      break;

    case 3:
      if (BOF3_SCENA16_DAT_80143c40 != 0u) {
        func_8014f800(0x50, 0x50, 0, 0xffu,
                      0x80010000u + (u32)BOF3_SCENA16_DAT_80010004);
        break;
      }

      func_8014ecac(1u);
      BOF3_SCENA16_DAT_80146866 = 0x30u;
      BOF3_SCENA16_DAT_8014832e = 0x1fu;
      *(vu8*)0x1f800000u = func_8019601c();

      if (*(vu8*)0x1f800000u != 0xffu) {
        vu8* object;
        u32  object_index;

        object_index = (u32) * (vu8*)0x1f800000u;
        object = (vu8*)(0x80143fc8u + (object_index * 0x74u));
        object[0] = 1u;
        object[5] = 0x13u;
        *(vs32*)(object + 0x64) = (s32)((s16)BOF3_SCENA16_DAT_801492d8 + 0x1c0);
        *(vs32*)(object + 0x68) = (s32)(s16)BOF3_SCENA16_DAT_801492da;
        *(vs32*)(object + 0x6c) = (s32)(s16)BOF3_SCENA16_DAT_801492dc;
        object[9] = 0xffu;
      }

      goto increment_state;

    case 4:
      if (BOF3_SCENA16_DAT_80143c40 == 0u) {
        BOF3_SCENA16_DAT_80146876 = 0u;
        goto increment_state;
      }
      break;

    case 5:
      func_8014f800(0x48, 0x50, 0, 0xffu,
                    0x80010000u + (u32)BOF3_SCENA16_DAT_80010006);
      timer = (vu16*)0x80146876u;
      *timer = (u16)(*timer + 1u);
      if (*timer != 0x20u) {
        func_801f83b0((u32)((u8)*timer));
        break;
      }
      func_801f845c();
      *timer = 0x7fu;
      goto increment_state;

    case 6:
      func_8014f800(0x48, 0x50, 0, 0xffu,
                    0x80010000u + (u32)BOF3_SCENA16_DAT_80010006);
      BOF3_SCENA16_DAT_80146876 = (u16)(BOF3_SCENA16_DAT_80146876 - 1u);
      if (BOF3_SCENA16_DAT_80146876 == 0u) {
        BOF3_SCENA16_DAT_80146876 = 0x20u;
        goto increment_state;
      }
      break;

    case 7:
      timer = (vu16*)0x80146876u;
      *timer = (u16)(*timer - 1u);
      if (*timer != 0u) {
        func_8014f800(0x48, 0x50, 0, 0xffu,
                      0x80010000u + (u32)BOF3_SCENA16_DAT_80010006);
        func_801f83b0((u32)(*(vu8*)timer));
        break;
      }
      func_801f845c();
      *timer = 0x1eu;
      goto increment_state;

    case 8:
      BOF3_SCENA16_DAT_80146876 = (u16)(BOF3_SCENA16_DAT_80146876 - 1u);
      if (BOF3_SCENA16_DAT_80146876 == 0u) {
        goto increment_state;
      }
      break;

    case 9:
      func_80161c20(2u, 100, 0x20);
      func_8019fa28(2u, 0x340000u, 0x430000u, 3u);
      BOF3_SCENA16_DAT_80146874 = 4;
      BOF3_SCENA16_DAT_80146875 = 0u;
      break;
  }

footer:
  if (BOF3_SCENA16_DAT_80149314 == 0x4400u) {
    BOF3_SCENA16_DAT_80149314 = 0x6200u;
    BOF3_SCENA16_DAT_80149322 = (u16)(BOF3_SCENA16_DAT_80149322 - 0x1eu);
    BOF3_SCENA16_DAT_8014930c += -0x1e0000;
    BOF3_SCENA16_DAT_80147a90 += -0x1e0000;
    BOF3_SCENA16_DAT_80143f80 += -0x1e0000;
    func_80154698();
  }

  return;

increment_state:
  BOF3_SCENA16_DAT_80146875 = (u8)(BOF3_SCENA16_DAT_80146875 + 1u);
  goto footer;
}
