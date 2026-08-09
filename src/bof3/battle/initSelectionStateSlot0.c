#include "bof3/battle/battle15_internal.h"

/* @source 0x80098254
 * @behavior initializes battle selection/action state via setup mode 0x104, selects slot 0, sets active selection flags, resets state, and increments the counter.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initSelectionStateSlot0(void) {
  u8 counter;

  func_8015DF18(0x104);
  D_801462EF = 0;
  BATTLE_ACTIVE_SELECTION_SLOT_PTR[1] = 0;
  *(u32*)(BATTLE_ACTIVE_SELECTION_SLOT_PTR + 0xC) |= 1;
  D_801EBF08_PTR->unk_01 = 2;
  counter = D_80146303;
  D_801462E1 = 1;
  D_801462E2 = 0;
  D_801462E3 = 0;
  D_80146303 = counter + 1;
}
