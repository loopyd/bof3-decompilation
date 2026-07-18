#include "internal.h"

/* @behavior updates the world-front position and enters main state 2 when it
 * reaches coordinates (-682, 512).
 * @source 0x801981D4
 */
void func_801981D4(void) {
  func_801990D0();
  if (D_801492D8 == -682 && D_801492DC == 512) {
    D_80143BB0 = 0;
    D_80143B90 = 2;
    D_80143B92 = 0;
  }
}
