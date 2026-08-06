#include "internal.h"

/* @source 0x80096B24
 * @behavior Stores selection slot 3, resets selection state, and advances phase.
 */
void storeSlot3AdvanceSubstate(void) {
  u8 value;

  *D_801EB4D8 = func_801DB5CC(3);
  value = D_801462E4;
  D_801462EF = 1;
  D_80145AC8 = 0;
  D_801462E4 = value + 1;
}
