#include "internal.h"

/* @behavior clears the 16-bit count, sets state byte 2, returns the
 * constant 2.
 * @source 0x801F455C
 */
s32 func_801F455C(void) {
  s32 state = 2;

  D_80149328 = 0;
  D_80149333 = (u8)state;
  return state;
}
