#include "internal.h"

/* @behavior updates the world-front position and enters main state 2 when it
 * reaches coordinates (-682, 512).
 * @source 0x801981d4 func_801981d4
 */
void func_801981d4(void) {
  func_801990d0();
  if (DAT_801492d8 == -682 && DAT_801492dc == 512) {
    DAT_80143bb0 = 0;
    DAT_80143b90 = 2;
    DAT_80143b92 = 0;
  }
}
