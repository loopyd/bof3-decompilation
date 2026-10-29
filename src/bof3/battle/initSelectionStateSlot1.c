#include "bof3/battle/battle15_internal.h"

/* @source 0x80096E14
 * @behavior Calls 0x8015DF18(0x104), clears byte 0x801462EF, stores 1 at byte +0x01
 * through the pointer at 0x801EB4D8 and 2 at byte +0x01 through the pointer at
 * 0x801EBF08, sets byte 0x801462E1 to 1, clears bytes 0x801462E2 through
 * 0x801462E4, and increments byte 0x80146303 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initSelectionStateSlot1(void) {
  u8 counter;

  func_8015DF18(0x104);
  D_801462EF = 0;
  BATTLE_ACTIVE_SELECTION_SLOT_PTR[1] = 1;
  D_801EBF08_PTR->unk_01 = 2;
  counter = D_80146303;
  D_801462E1 = 1;
  D_801462E2 = 0;
  D_801462E3 = 0;
  D_801462E4 = 0;
  D_80146303 = counter + 1;
}
