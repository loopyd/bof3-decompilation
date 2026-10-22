#include "bof3/scenario/scena16_internal.h"

/* Same shape as the exact scena00 sibling func_801FA27C and the exact
 * same-target func_801F7790: the switch on D_80146875 is the original
 * thirteen-entry absolute jump table at 0x801F6C5C and the 0x74-byte row is
 * seeded through one extern base per field.
 *
 * Three original shapes are kept verbatim because the original bytes prove
 * them: states 0 and 2 share one tail (the original loads the mode argument
 * before the branch and enters a single func_80150224 / D_80143BB0 = 2 block)
 * and reach it through a label; the states that advance through a scratch
 * pointer address of D_80146875 are 1, 5 and 9, while the shared tail and the
 * remaining states use the plain store wording; and the block-local pointer in
 * the shared tail is what keeps that increment separate from the state 1/5/9
 * one (a function-scope pointer lets the compiler cross-jump the two together
 * and drops four instructions).
 */

/* @behavior Runs the secondary SCENA16 controller rooted at state 4 through the
 * thirteen-entry absolute jump table at 0x801F6C5C, keyed by the state byte
 * D_80146875, and advances that byte by one on the states that finish: state 0,
 * while the gate word D_80143C40 is clear, queues front-end mode 1 in
 * func_80150224, arms D_80143BB0 = 2 and advances; state 1 advances once that
 * mode byte leaves 2; state 2 waits for scenario progress byte 3, re-queues mode
 * 2 and re-arms D_80143BB0; state 3 waits for mode byte 2 to drop and progress
 * byte 4, then reloads the 0x130-frame countdown D_80146876; state 4 publishes
 * 0x130 - D_80146876 into the scratchpad halfword 0x1F800000 and, while that
 * value stays below 0xBF, draws the 0x48/0x50 window from D_80010006 over the
 * shared VRAM base and clamps the palette channels with that value (below 0x20
 * with the value itself, from 0xA0 up with 0xBF - value, and copies the palette
 * block at exactly 0x20 or 0xBF), then counts the countdown down and publishes
 * ((0x130 - remaining) * 0x57943) >> 16 in D_8014932C while it runs; state 5, on
 * progress byte 5, publishes func_8019601C() in scratchpad 0x1F800000 and,
 * unless that byte is 0xFF, seeds the 0x74-byte row D_80143FC8 + index * 0x74
 * (+0 = 1, +5 = 0x13, the word -0x34A at +0x64, the s16 words D_801492DA and
 * D_801492DC at +0x68/+0x6C, +9 = 0x60); state 6 repeats that seeding on
 * progress byte 8 (-0x2AC at +0x64, +9 = 0x20) and arms the 0x200-frame
 * countdown; state 7 counts that countdown down, publishing (remaining * 0xD) >>
 * 2 in D_8014932C while it runs; state 8, on progress byte 0xB, runs
 * func_8014ECAC(0) and clears the countdown; state 9, while the gate word is
 * clear, clears the effect bank D_8014832E, runs func_8014ECAC(1) and queues
 * func_80161CD0(2, 0x6E, 0x20); state 10 draws that window and, while the gate
 * word stays clear, reloads 0x7F into the countdown; state 11 keeps drawing that
 * window while the countdown runs and runs func_8014ECAC(0) when it reaches
 * zero; state 12 either requests keyed state 1 in D_80146874 and returns the
 * state byte to 0, or re-draws that window while the gate word is set. The
 * 0x74-byte row writes re-read the published index byte from scratchpad.
 * @source 0x801F7CC4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7CC4(void) {
  u8* next;
  u32 mode;

  switch (D_80146875) {
    case 0:
      if (D_80143C40 != 0u) {
        break;
      }

      mode = 1u;
      goto seal;

    case 1:
      if (D_80143BB0 == 2u) {
        break;
      }

      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;

    case 2:
      if (*(u8 *)&g_ScenarioProgress != 3u) {
        break;
      }

      mode = 2u;
    seal: {
      u8* state;

      func_80150224(mode);
      D_80143BB0 = 2u;
      state = &D_80146875;
      *state = (u8)(*state + 1u);
      break;
    }

    case 3:
      if (D_80143BB0 == 2u) {
        break;
      }
      if (*(u8 *)&g_ScenarioProgress != 4u) {
        break;
      }

      D_80146876 = 0x130u;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;

    case 4: {
      u16* published;
      u16* counter;

      published = (u16*)&D_1F800000;
      *published = (u16)(0x130u - D_80146876);
      if (*published < 0xbfu) {
        func_8014F800(0x48, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010006);
        if (*published < 0x20u) {
          clampPaletteChannels((u32)((u8)*published));
        } else if (*published == 0x20u) {
          copyPaletteBlock();
        } else if (*published >= 0xa0u) {
          clampPaletteChannels((u32)((u8)(0xbfu - *published)));
        }
      } else if (*published == 0xbfu) {
        copyPaletteBlock();
      }

      counter = &D_80146876;
      if (*counter != 0u) {
        u16 remaining = (u16)(*counter - 1u);

        *counter = remaining;
        D_8014932C = (u16)(((0x130u - (u32)remaining) * 0x57943u) >> 16);
        break;
      }

      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 5: {
      u8* scratch;
      u8  object_index;

      if (*(u8 *)&g_ScenarioProgress != 5u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x34a;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x60u;
      }

      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;
    }

    case 6: {
      u8* scratch;
      u8  object_index;

      if (*(u8 *)&g_ScenarioProgress != 8u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = -0x2ac;
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x20u;
      }

      D_80146876 = 0x200u;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 7: {
      u16* counter;
      u32  count;

      counter = &D_80146876;
      count = (u32)*counter;
      if (count != 0u) {
        u16 remaining = (u16)(*counter - 1u);

        D_8014932C = (u16)((count * 0xdu) >> 2);
        *counter = remaining;
        break;
      }

      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 8:
      if (*(u8 *)&g_ScenarioProgress != 0xbu) {
        break;
      }

      func_8014ECAC(0u);
      D_80146876 = 0u;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;

    case 9:
      if (D_80143C40 != 0u) {
        break;
      }

      D_8014832E = 0u;
      func_8014ECAC(1u);
      func_80161CD0(2u, 0x6e, 0x20);
      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;

    case 10:
      func_8014F800(0x46, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010008);
      if (D_80143C40 == 0u) {
        D_80146876 = 0x7fu;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;

    case 11: {
      u16* counter;

      func_8014F800(0x46, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010008);
      counter = &D_80146876;
      *counter = (u16)(*counter - 1u);
      if (*counter != 0u) {
        break;
      }

      func_8014ECAC(0u);
      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 12:
      if (D_80143C40 == 0u) {
        D_80146874 = 1;
        D_80146875 = 0u;
        break;
      }

      func_8014F800(0x46, 100, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010008);
      break;
  }
}
