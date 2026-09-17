#include "bof3/scenario/scena16_internal.h"

/* @behavior runs the secondary SCENA16 controller rooted at state 4.
 * @source 0x801F7CC4
 * @status partial
 * @match 55.24
 * @residual 237 matching instructions; 429 current/421 original; 1684 original bytes versus 1716 current.
 */
void func_801F7CC4(void) {
  u8 advance_state;
  u8 object_index;

  advance_state = 0u;

  switch (D_80146875) {
    case 0:
      if (D_80143C40 != 0u) {
        return;
      }
      func_80150224(1u);
      g_GameState = 2u;
      advance_state = 1u;
      break;

    case 1:
      if (g_GameState == 2u) {
        return;
      }
      advance_state = 1u;
      break;

    case 2:
      if (g_ScenarioProgress != 3u) {
        return;
      }
      func_80150224(2u);
      g_GameState = 2u;
      advance_state = 1u;
      break;

    case 3:
      if (g_GameState != 2u && g_ScenarioProgress == 4u) {
        D_80146876 = 0x130u;
        advance_state = 1u;
      }
      break;

    case 4: {
      u16 progress;
      u16 next_counter;

      progress = (u16)(0x130u - D_80146876);
      if (progress < 0xbfu) {
        func_8014F800(0x48, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010006);
        if (progress < 0x20u) {
          clampPaletteChannels((u32)(progress & 0xffu));
        } else if (progress == 0x20u) {
          copyPaletteBlock();
        } else if (progress > 0x9fu) {
          clampPaletteChannels((u32)((0xbfu - progress) & 0xffu));
        }
      } else if (progress == 0xbfu) {
        copyPaletteBlock();
      }

      next_counter = (u16)(D_80146876 - 1u);
      if (D_80146876 != 0u) {
        D_80146876 = next_counter;
        D_8014932C =
            (u16)((((u32)(0x130u - (u32)next_counter)) * 0x57943u) >> 16);
        return;
      }

      advance_state = 1u;
      break;
    }

    case 5:
      if (g_ScenarioProgress != 5u) {
        return;
      }
      object_index = func_8019601C();
      SPAD_REF(volatile u8, 0x0u) = object_index;
      if (object_index != 0xffu) {
        volatile u8* object;

        object = PSX_PTR(volatile u8, 0x80143fc8u) + ((u32)object_index * 0x74u);
        object[0] = 1u;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        object[5] = 0x13u;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x64) = -0x34a;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x68) = (s32)(s16)D_801492DA;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x6c) = (s32)(s16)D_801492DC;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        object[9] = 0x60u;
      }
      advance_state = 1u;
      break;

    case 6:
      if (g_ScenarioProgress != 8u) {
        break;
      }
      object_index = func_8019601C();
      SPAD_REF(volatile u8, 0x0u) = object_index;
      if (object_index != 0xffu) {
        volatile u8* object;

        object = PSX_PTR(volatile u8, 0x80143fc8u) + ((u32)object_index * 0x74u);
        object[0] = 1u;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        object[5] = 0x13u;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x64) = -0x2ac;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x68) = (s32)(s16)D_801492DA;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        *(volatile s32*)(object + 0x6c) = (s32)(s16)D_801492DC;
        object = PSX_PTR(volatile u8, 0x80143fc8u) +
                 ((u32)SPAD_REF(volatile u8, 0x0u) * 0x74u);
        object[9] = 0x20u;
      }
      D_80146876 = 0x200u;
      advance_state = 1u;
      break;

    case 7: {
      u32 counter;

      counter = (u32)D_80146876;
      if (counter != 0u) {
        D_80146876 = (u16)(D_80146876 - 1u);
        D_8014932C = (u16)((counter * 0xdu) >> 2);
        return;
      }

      advance_state = 1u;
      break;
    }

    case 8:
      if (g_ScenarioProgress == 0x0bu) {
        func_8014ECAC(0u);
        D_80146876 = 0u;
        advance_state = 1u;
      }
      break;

    case 9:
      if (D_80143C40 != 0u) {
        return;
      }
      D_8014832E = 0u;
      func_8014ECAC(1u);
      func_80161CD0(2u, 0x6e, 0x20);
      advance_state = 1u;
      break;

    case 10:
      func_8014F800(0x46, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010008);
      if (D_80143C40 == 0u) {
        D_80146876 = 0x7fu;
        advance_state = 1u;
      }
      break;

    case 11:
      func_8014F800(0x46, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010008);
      D_80146876 = (u16)(D_80146876 - 1u);
      if (D_80146876 != 0u) {
        return;
      }
      func_8014ECAC(0u);
      advance_state = 1u;
      break;

    case 12:
      if (D_80143C40 == 0u) {
        D_80146874 = 1;
        D_80146875 = 0u;
      } else {
        func_8014F800(0x46, 100, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010008);
      }
      break;
  }

  if (advance_state != 0u) {
    D_80146875 = (u8)(D_80146875 + 1u);
  }
}
