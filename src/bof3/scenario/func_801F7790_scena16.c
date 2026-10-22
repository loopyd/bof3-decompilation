#include "bof3/scenario/scena16_internal.h"

/* The 0x74-byte row at D_80143FC8 is seeded through one extern base per field
 * (D_80143FCD/D_80143FD1 and the word bases D_8014402C/D_80144030/D_80144034)
 * because the original re-materialises `lui at,0x8014` for every statement: a
 * single struct base lets the compiler fold the trailing stores into one
 * base+offset chain. Same spelling as the exact scena00 row seeders.
 */

/* @behavior Runs the secondary SCENA16 controller selected by the state byte
 * D_80146875 through a ten-entry absolute jump table: state 0, while the gate
 * word D_80143C40 is clear, runs func_8014ECAC(1) and advances; state 1 draws
 * the 0x50/0x50 window from D_80010004 over the shared VRAM base and, while the
 * gate word is clear, reloads 0x7F into D_80146876 and advances; state 2
 * re-draws that window, counts D_80146876 down and at zero runs
 * func_8014ECAC(0) before advancing; state 3 keeps re-drawing that window while
 * the gate word is set and otherwise runs func_8014ECAC(1), stores 0x30 in
 * D_80146866, arms effect bank 0x1F in D_8014832E, publishes func_8019601C() in
 * scratchpad 0x1F800000 and, unless that byte is 0xFF, seeds the 0x74-byte row
 * D_80143FC8 + index * 0x74 (+0 = 1, +5 = 0x13, the signed words D_801492D8 +
 * 0x1C0, D_801492DA and D_801492DC at +0x64/+0x68/+0x6C, +9 = 0xFF) before
 * advancing; state 4, while the gate word is clear, clears D_80146876 and
 * advances; state 5 draws the 0x48/0x50 window from D_80010006, counts
 * D_80146876 up and, at 0x20, copies the palette block, reloads 0x7F and
 * advances; at other values it clamps the palette channels with the count.
 * State 6 re-draws that window, counts D_80146876 down and at zero
 * reloads 0x20 and advances; state 7 counts D_80146876 down and, above zero,
 * re-draws that window and clamps the palette channels with the fresh count,
 * otherwise copies the palette block, reloads 0x1E and advances; state 8 counts
 * D_80146876 down and advances at zero; state 9 runs func_80161C20(2, 100,
 * 0x20), seeds the front selector context func_8019FA28(2, 0x340000, 0x430000,
 * 3), requests secondary state 4 in D_80146874 and returns the state byte to 0.
 * The advancing states either increment D_80146875 through its address or
 * through one %hi/%lo store, and the 0x74-byte row writes re-read the index
 * byte from scratchpad.
 * @source 0x801F7790
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7790(void) {
  u8* next;

  switch (D_80146875) {
    case 0:
      if (D_80143C40 != 0u) {
        break;
      }

      func_8014ECAC(1u);
      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;

    case 1:
      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010004);
      if (D_80143C40 == 0u) {
        D_80146876 = 0x7fu;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;

    case 2: {
      u16* timer;

      func_8014F800(0x50, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010004);
      timer = &D_80146876;
      *timer = (u16)(*timer - 1u);
      if (*timer == 0u) {
        func_8014ECAC(0u);
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;
    }

    case 3:
      if (D_80143C40 != 0u) {
        func_8014F800(0x50, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010004);
        break;
      }

      func_8014ECAC(1u);
      D_80146866 = 0x30u;
      D_8014832E = 0x1fu;

      {
        u8* scratch;
        u8  object_index;

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xffu) {
          D_80143FC8[(u32)object_index * 0x74u] = 1u;
          D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
          *(s32*)(D_8014402C + (u32)*scratch * 0x74u) =
              (s32)((s16)D_801492D8 + 0x1c0);
          *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
          *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
          D_80143FD1[(u32)*scratch * 0x74u] = 0xffu;
        }
      }

      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;

    case 4:
      if (D_80143C40 != 0u) {
        break;
      }

      D_80146876 = 0u;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;

    case 5: {
      u16* timer;
      u16  count;

      func_8014F800(0x48, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010006);
      timer = &D_80146876;
      count = (u16)(*timer + 1u);
      *timer = count;
      if (count == 0x20u) {
        copyPaletteBlock();
        *timer = 0x7fu;
        D_80146875 = (u8)(D_80146875 + 1u);
      } else {
        clampPaletteChannels((u32)((u8)count));
      }
      break;
    }

    case 6: {
      u16* timer;

      func_8014F800(0x48, 0x50, 0, 0xffu,
                    SCENA16_VRAM_BASE + (u32)D_80010006);
      timer = &D_80146876;
      *timer = (u16)(*timer - 1u);
      if (*timer == 0u) {
        *timer = 0x20u;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;
    }

    case 7: {
      u16* timer;

      timer = &D_80146876;
      *timer = (u16)(*timer - 1u);
      if (*timer != 0u) {
        func_8014F800(0x48, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010006);
        clampPaletteChannels((u32)((u8)*timer));
        break;
      }
      copyPaletteBlock();
      *timer = 0x1eu;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 8: {
      u16* timer;

      timer = &D_80146876;
      *timer = (u16)(*timer - 1u);
      if (*timer != 0u) {
        break;
      }

      D_80146875 = (u8)(D_80146875 + 1u);
      break;
    }

    case 9:
      func_80161C20(2u, 100, 0x20);
      func_8019FA28(2u, 0x340000u, 0x430000u, 3u);
      D_80146874 = 4;
      D_80146875 = 0u;
      break;
  }

  if (D_80149314 == 0x4400u) {
    s32* word0;
    s32* word1;
    s32* word2;

    D_80149314 = 0x6200u;
    word0 = &D_8014930C;
    word1 = &D_80147A90;
    word2 = &D_80143F80;
    D_80149322 = (u16)(D_80149322 - 0x1eu);
    *word0 += -0x1e0000;
    *word1 += -0x1e0000;
    *word2 += -0x1e0000;
    func_80154698();
  }
}
