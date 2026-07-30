#include "internal.h"

/* @source 0x80096E14 */
/* @behavior Initializes battle selection/action state, calls setup mode 0x104, resets relevant state bytes, selects slot 1, and increments the counter. */
void func_80096E14(void) {
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
