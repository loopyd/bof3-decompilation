#include "internal.h"

/* @behavior writes a fixed mode byte then advances the tracked u16 counter by
 * a fixed stride and stores it back.
 * @source 0x801F4578
 */
void func_801F4578(void) {
  u16 count;

  count = WORLD00_AREA008_D_8014932A;
  WORLD00_AREA008_D_80149333 = 2;
  WORLD00_AREA008_D_8014932A = (u16)(count + 0x14);
}
