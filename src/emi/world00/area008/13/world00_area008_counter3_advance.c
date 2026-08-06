#include "internal.h"

/* @source 0x801F45E4
 * @behavior increments the area counter by 0x800
 */
void world00_area008_counter3_advance(void) {
  s32 *counter;
  s32 value;

  counter = &world00_area008_counter3;
  value = *counter;
  value += 0x800;
  *counter = value;
}
