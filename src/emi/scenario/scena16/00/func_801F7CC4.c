#include "internal.h"

/* @behavior runs the secondary SCENA16 controller rooted at state 4.
 * @source 0x801F7CC4
 */
void func_801F7CC4(void) {
  u8 advance_state;

  advance_state = 0u;

  switch (SCENA16_D_80146875) {
    case 0:
      if (SCENA16_D_80143C40 != 0u) {
        return;
      }
      func_80150224(1u);
      SCENA16_D_80143BB0 = 2u;
      advance_state = 1u;
      break;

    case 1:
      if (SCENA16_D_80143BB0 == 2u) {
        return;
      }
      advance_state = 1u;
      break;

    case 2:
      if (SCENA16_D_80146864_BYTE != 3u) {
        return;
      }
      func_80150224(2u);
      SCENA16_D_80143BB0 = 2u;
      advance_state = 1u;
      break;

    case 3:
      if (SCENA16_D_80143BB0 != 2u && SCENA16_D_80146864_BYTE == 4u) {
        SCENA16_D_80146876 = 0x130u;
        advance_state = 1u;
      }
      break;

    case 4: {
      u16 progress;
      u16 next_counter;

      progress = (u16)(0x130u - SCENA16_D_80146876);
      if (progress < 0xbfu) {
        func_8014F800(0x48, 0x50, 0, 0xffu,
                      0x80010000u + (u32)SCENA16_D_80010006);
        if (progress < 0x20u) {
          func_801F83B0((u32)(progress & 0xffu));
        } else if (progress == 0x20u) {
          func_801F845C();
        } else if (progress > 0x9fu) {
          func_801F83B0((u32)((0xbfu - progress) & 0xffu));
        }
      } else if (progress == 0xbfu) {
        func_801F845C();
      }

      next_counter = (u16)(SCENA16_D_80146876 - 1u);
      if (SCENA16_D_80146876 != 0u) {
        SCENA16_D_80146876 = next_counter;
        SCENA16_D_8014932C =
            (u16)((((u32)(0x130u - (u32)next_counter)) * 0x57943u) >> 16);
        return;
      }

      advance_state = 1u;
      break;
    }

    case 5:
      if (SCENA16_D_80146864_BYTE != 5u) {
        return;
      }
      *(vu8*)0x1f800000u = func_8019601C();
      if (*(vu8*)0x1f800000u != 0xffu) {
        vu8* object;
        u32  object_index;

        object_index = (u32) * (vu8*)0x1f800000u;
        object = (vu8*)(0x80143fc8u + (object_index * 0x74u));
        object[0] = 1u;
        object[5] = 0x13u;
        *(vs32*)(object + 0x64) = -0x34a;
        *(vs32*)(object + 0x68) = (s32)(s16)SCENA16_D_801492DA;
        *(vs32*)(object + 0x6c) = (s32)(s16)SCENA16_D_801492DC;
        object[9] = 0x60u;
      }
      advance_state = 1u;
      break;

    case 6:
      if (SCENA16_D_80146864_BYTE != 8u) {
        break;
      }
      *(vu8*)0x1f800000u = func_8019601C();
      if (*(vu8*)0x1f800000u != 0xffu) {
        vu8* object;
        u32  object_index;

        object_index = (u32) * (vu8*)0x1f800000u;
        object = (vu8*)(0x80143fc8u + (object_index * 0x74u));
        object[0] = 1u;
        object[5] = 0x13u;
        *(vs32*)(object + 0x64) = -0x2ac;
        *(vs32*)(object + 0x68) = (s32)(s16)SCENA16_D_801492DA;
        *(vs32*)(object + 0x6c) = (s32)(s16)SCENA16_D_801492DC;
        object[9] = 0x20u;
      }
      SCENA16_D_80146876 = 0x200u;
      advance_state = 1u;
      break;

    case 7: {
      u32 counter;

      counter = (u32)SCENA16_D_80146876;
      if (counter != 0u) {
        SCENA16_D_80146876 = (u16)(SCENA16_D_80146876 - 1u);
        SCENA16_D_8014932C = (u16)((counter * 0xdu) >> 2);
        return;
      }

      advance_state = 1u;
      break;
    }

    case 8:
      if (SCENA16_D_80146864_BYTE == 0x0bu) {
        func_8014ECAC(0u);
        SCENA16_D_80146876 = 0u;
        advance_state = 1u;
      }
      break;

    case 9:
      if (SCENA16_D_80143C40 != 0u) {
        return;
      }
      SCENA16_D_8014832E = 0u;
      func_8014ECAC(1u);
      func_80161CD0(2u, 0x6e, 0x20);
      advance_state = 1u;
      break;

    case 10:
      func_8014F800(0x46, 100, 0, 0xffu, 0x80010000u + (u32)SCENA16_D_80010008);
      if (SCENA16_D_80143C40 == 0u) {
        SCENA16_D_80146876 = 0x7fu;
        advance_state = 1u;
      }
      break;

    case 11:
      func_8014F800(0x46, 100, 0, 0xffu, 0x80010000u + (u32)SCENA16_D_80010008);
      SCENA16_D_80146876 = (u16)(SCENA16_D_80146876 - 1u);
      if (SCENA16_D_80146876 != 0u) {
        return;
      }
      func_8014ECAC(0u);
      advance_state = 1u;
      break;

    case 12:
      if (SCENA16_D_80143C40 == 0u) {
        SCENA16_D_80146874 = 1;
        SCENA16_D_80146875 = 0u;
      } else {
        func_8014F800(0x46, 100, 0, 0xffu,
                      0x80010000u + (u32)SCENA16_D_80010008);
      }
      break;
  }

  if (advance_state != 0u) {
    SCENA16_D_80146875 = (u8)(SCENA16_D_80146875 + 1u);
  }
}
