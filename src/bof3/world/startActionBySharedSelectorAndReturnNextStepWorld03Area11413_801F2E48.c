#include "bof3/bof3.h"

/* @source 0x801454EC @kind unknown */
extern u8 D_801454EC;
/* @source 0x80144F28 @kind unknown */
extern u8 D_80144F28;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(void* arg0, u8 bit_index);
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
void func_80150224(s32 arg0);
u32 func_80166140(u32 arg0);
void func_80166180(s32 arg0);

/* @source 0x801F2E48
 * @behavior Reads the shared selector byte D_801454EC and switches on it.
 *           Selector 0 and any unhandled value start action 1 and report the
 *           next step as 1. Selectors 0xA, 0xB and 0xC read flag bits 0x96 and
 *           0x97 of the main-RAM flag bank D_80144F28 through the shared flag
 *           tester func_8015B5D4: a bit that is already set starts action 5 and
 *           reports step 1, while a clear bit runs the func_801650B4 gate
 *           (message 0x2D with three options, or 0x33 with one), sets that bit
 *           through func_8015B580 and selects a per-selector action - 2 for 0xA
 *           at step 5, 6 for 0xB at step 5 or 3 once its second bit is clear
 *           too, and 8, 7 or 4 for 0xC after the func_80166140/func_80166180
 *           pair on id 0xF opens that arm, at step 10. Every path then starts
 *           its action through func_80150224, latches the phase byte
 *           D_80143BB0 to 2 and returns the next step.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startActionBySharedSelectorAndReturnNextStepWorld03Area11413_801F2E48(void) {
  s32 action;
  s32 state;

  switch (D_801454EC) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    default:
      action = 1;
      state = 1;
      break;
    case 10: {
      u8* bank;
      bank = &D_80144F28;
      if (func_8015B5D4((u32)bank, 0x96) == 0) {
        func_801650B4(3, 0x2D, 3, 0);
        func_8015B580(bank, 0x96);
        action = 2;
        state = 5;
      } else {
        action = 5;
        state = 1;
      }
      break;
    }
    case 11: {
      u8* bank;
      bank = &D_80144F28;
      if (func_8015B5D4((u32)bank, 0x97) == 0) {
        func_801650B4(3, 0x33, 1, 0);
        func_8015B580(bank, 0x97);
        action = 6;
        state = 5;
        if (func_8015B5D4((u32)bank, 0x96) == 0) {
          func_801650B4(3, 0x2D, 3, 0);
          func_8015B580(bank, 0x96);
          action = 3;
        }
      } else {
        action = 5;
        state = 1;
      }
      break;
    }
    case 12: {
      u8* bank;
      if (func_80166140(0xF) == 0) {
        func_80166180(0xF);
        action = 8;
        bank = &D_80144F28;
        if (func_8015B5D4((u32)bank, 0x97) == 0) {
          func_801650B4(3, 0x33, 1, 0);
          func_8015B580(bank, 0x97);
          action = 7;
          if (func_8015B5D4((u32)bank, 0x96) == 0) {
            func_801650B4(3, 0x2D, 3, 0);
            func_8015B580(bank, 0x96);
            action = 4;
          }
        }
        state = 10;
      } else {
        action = 5;
        state = 1;
      }
      break;
    }
  }

  func_80150224(action);
  D_80143BB0 = 2;
  return state;
}
