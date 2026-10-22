#include "bof3/scenario/scena16_internal.h"

/* The 0x74-byte row at D_80143FC8 is seeded through one extern base per field
 * (D_80143FCD/D_80143FD1 and the word bases D_8014402C/D_80144030/D_80144034)
 * because the original re-materialises `lui at,0x8014` for every statement: a
 * single struct base lets the compiler fold the trailing stores into one
 * base+offset chain. Same spelling as the exact scena00 row seeders.
 */

/* @behavior Runs the secondary SCENA16 controller selected by the state byte
 * D_80146875 through a twelve-entry absolute jump table: state 0, while the gate
 * word D_80143C40 is clear, runs func_8014ECAC(1), func_8016C0C0(0, 0) and
 * func_80161C20(6, 0x50, 8), raises flag 0x80 in the mode word D_80146258 and
 * advances; state 1 draws the 0x7A/0x64 window from D_80010020 over the shared
 * VRAM base and, while the gate word is clear, clears D_80146876 and advances;
 * state 2 counts D_80146876 up, re-draws that window and, at 0x7F, runs
 * func_8014ECAC(0), clears the counter and advances; state 3 keeps re-drawing
 * that window while the gate word is set and otherwise arms effect bank 0x1F in
 * D_8014832E, runs func_8015C088/func_8015C100/func_8014ECAC(1), stores 1 in
 * D_80146867, clears D_80146876 and advances; state 4 at D_80146867 = 0x54 runs
 * func_8014ECAC(0) and advances; state 5, while the gate word is clear, clears
 * D_8014832E, runs func_8014ECAC(1) and advances; state 6 draws the 0x50/0x50
 * window from D_80010022 and, while the gate word is clear, reloads 0x7F into
 * D_80146876 and advances; state 7 re-draws that window, counts D_80146876 down
 * and at zero runs func_8014ECAC(0) and advances; state 8 draws the second window
 * while the gate word is set and otherwise seeds the front selector context
 * func_8019FA28(4, 0x440000, 0x80000, 5) before advancing; state 9, at
 * D_80143F03 = 2, arms D_8014832E = 0x1F and advances; state 10, at
 * D_80146866 = 0x23, publishes func_8019601C() in scratchpad 0x1F800000 and,
 * unless that byte is 0xFF, seeds the 0x74-byte row D_80143FC8 + index * 0x74
 * (+0 = 1, +5 = 0x13, the signed words D_801492D8 + 0x180, D_801492DA and
 * D_801492DC at +0x64/+0x68/+0x6C, +9 = 0x60) before advancing; state 11, at
 * D_80146867 = 0x60, seeds the front selector context
 * func_8019FA28(0x1F, 0x70000, 0x140000, 0), stores 0xFF in D_80143F1D, runs
 * func_80161CD0(6, 0x50, 0x40), clears D_80146866 and D_80146875 and sets
 * D_80146874 to 3. States 0 through 10 advance D_80146875; states 0, 4, 5,
 * 8, 9 and 10 use its address. The row writes re-read the scratchpad index.
 * @source 0x801F7230
 * @status partial
 * @match 98.84
 * @residual The shared state-increment tail uses a0 instead of v1 for its byte pointer; four instructions differ at unchanged 1376-byte size.
 */
void func_801F7230(void) {
  u8* next;

  switch (D_80146875) {
    case 0: {
      u8 step;
      u16 flags;

      if (D_80143C40 == 0u) {
        func_8014ECAC(1u);
        func_8016C0C0(0, 0);
        func_80161C20(6u, 0x50, 8);
        next = &D_80146875;
        step = *next;
        flags = D_80146258;
        step++;
        D_80146258 = flags | 0x80u;
        *next = step;
      }
      break;
    }

    case 1:
      func_8014F800(0x7a, 100, 0, 0xffu, SCENA16_VRAM_BASE + (u32)D_80010020);
      if (D_80143C40 == 0u) {
        D_80146876 = 0u;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;

    case 2: {
      u16* counter = (u16*)&D_80146876;

      *counter = (u16)(*counter + 1u);
      func_8016C0C0((s32)(s16)*counter, (s32)(s16)*counter);
      func_8014F800(0x7a, 100, 0, 0xffu, SCENA16_VRAM_BASE + (u32)D_80010020);
      if (*counter == 0x7fu) {
        func_8014ECAC(0u);
        *counter = 0u;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;
    }

    case 3:
      if (D_80143C40 != 0u) {
        func_8014F800(0x7a, 100, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010020);
        break;
      }
      D_8014832E = 0x1fu;
      func_8015C088();
      func_8015C100();
      func_8014ECAC(1u);
      D_80146867 = 1u;
      D_80146876 = 0u;
      D_80146875 = (u8)(D_80146875 + 1u);
      break;

    case 4:
      if (D_80146867 == 0x54u) {
        func_8014ECAC(0u);
        next = &D_80146875;
        *next = (u8)(*next + 1u);
      }
      break;

    case 5:
      if (D_80143C40 == 0u) {
        D_8014832E = 0u;
        func_8014ECAC(1u);
        next = &D_80146875;
        *next = (u8)(*next + 1u);
      }
      break;

    case 6:
      func_8014F800(0x50, 0x50, 0, 0xffu, SCENA16_VRAM_BASE + (u32)D_80010022);
      if (D_80143C40 == 0u) {
        D_80146876 = 0x7fu;
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;

    case 7: {
      u16* counter = (u16*)&D_80146876;

      func_8014F800(0x50, 0x50, 0, 0xffu, SCENA16_VRAM_BASE + (u32)D_80010022);
      *counter = (u16)(*counter - 1u);
      if (*counter == 0u) {
        func_8014ECAC(0u);
        D_80146875 = (u8)(D_80146875 + 1u);
      }
      break;
    }

    case 8:
      if (D_80143C40 != 0u) {
        func_8014F800(0x50, 0x50, 0, 0xffu,
                      SCENA16_VRAM_BASE + (u32)D_80010022);
        break;
      }
      func_8019FA28(4u, 0x440000u, 0x80000u, 5u);
      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;

    case 9: {
      u8* next;
      u8  step;

      if (D_80143F03 == 2u) {
        next = &D_80146875;
        step = *next;
        D_8014832E = 0x1fu;
        *next = (u8)(step + 1u);
      }
      break;
    }

    case 10: {
      u8* scratch;
      u8  object_index;

      if (D_80146866 != 0x23u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) =
            (s32)((s16)D_801492D8 + 0x180);
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x60u;
      }

      next = &D_80146875;
      *next = (u8)(*next + 1u);
      break;
    }

    case 11:
      if (D_80146867 == 0x60u) {
        func_8019FA28(0x1fu, 0x70000u, 0x140000u, 0u);
        D_80143F1D = 0xffu;
        func_80161CD0(6u, 0x50, 0x40);
        D_80146866 = 0u;
        D_80146874 = 3;
        D_80146875 = 0u;
      }
      break;
  }
}
