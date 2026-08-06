#include "internal.h"

/* @behavior sets state byte 2 and decrements the 16-bit count by 0x14.
 * @source 0x801F4534
 */
void world00_area008_counter1_retreat(void) {
  u16 count;

  count = world00_area008_counter1;
  D_80149333 = 2;
  world00_area008_counter1 = (u16)(count - 0x14);
}
