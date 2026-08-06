#include "internal.h"

/* @source 0x801F45C8
 * @behavior clears the state halfword and sets the adjacent state byte to 2
 */
void world00_area008_counter2_reset(void) {
  world00_area008_counter2 = 0;
  D_80149333 = 2;
}
