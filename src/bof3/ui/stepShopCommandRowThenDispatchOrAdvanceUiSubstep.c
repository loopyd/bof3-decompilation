#include "bof3/ui/shop00_internal.h"

/* @source 0x801E09F4
 * @behavior shop command-row sub-step handler reached as entry 6 of the
 *           sub-step table D_801E5CFC, which func_801DFDB8 dispatches with the
 *           UI sub-step byte D_80148652 as index: it republishes the selector
 *           frame (the ability-record halfword D_80148340 = 0x18, the x
 *           halfword D_80148628 = D_801483E8 + 5 and the y halfword
 *           D_8014862A = D_801483EA + 16 * D_801483EE + 0x1A), then steps the
 *           three-value row index D_801483EE with the direction bits of the
 *           pad state D_80145AA8 masked by 0x5000: bit 0x1000 decrements the
 *           index and wraps a decrement below zero back to 2, bit 0x4000
 *           increments it and wraps a count above two back to zero, and both
 *           arms play cue 0x101. When the pad state then shares the confirm
 *           mask D_80145AC2 it plays cue 0x103 and either, for the zero row,
 *           clears the selector flag D_801483EF and advances the UI sub-step
 *           byte D_80148652, or dispatches the row index as a command id
 *           through the shop command handler table; when it shares the cancel
 *           mask D_80145AC4 instead it plays cues 0x102 and 0x106, clears the
 *           mode byte D_801483E4, stores 4 in D_801483C3 and steps the UI
 *           phase byte D_80148651 back by two.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepShopCommandRowThenDispatchOrAdvanceUiSubstep(void) {
  s32 direction;
  s32 row;
  u8* step;
  u8 value;
  u8* phase;
  u8 next;

  D_80148340 = 0x18;
  D_80148628 = (u16)D_801483E8 + 5;
  D_8014862A = (u16)D_801483EA + (D_801483EE * 16 + 0x1A);
  direction = func_8014E640(D_80145AA8 & 0x5000);

  if (direction & 0x1000) {
    func_8015DF18(0x101);
    D_801483EE = D_801483EE - 1;
    if ((s8)D_801483EE < 0) {
      D_801483EE = 2;
    }
  } else if (direction & 0x4000) {
    func_8015DF18(0x101);
    D_801483EE = D_801483EE + 1;
    if (D_801483EE >= 3) {
      D_801483EE = 0;
    }
  }

  if (D_80145AA8 & D_80145AC2) {
    func_8015DF18(0x103);
    row = D_801483EE;
    if ((u8)row == 0) {
      step = &D_80148652;
      value = *step;
      D_801483EF = row;
      *step = value + 1;
    } else {
      dispatchCommand(row);
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x102);
    func_8015DF18(0x106);
    D_801483E4 = 0;
    phase = &D_80148651;
    next = *phase;
    D_801483C3 = 4;
    *phase = next - 2;
  }
}
