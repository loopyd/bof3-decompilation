#include "internal.h"

/* @source 0x801F45C8
 * @behavior clears the state halfword and sets the adjacent state byte to 2
 */
void func_801F45C8(void) {
  WORLD00_AREA008_D_8014932A = 0;
  WORLD00_AREA008_D_80149333 = 2;
}
