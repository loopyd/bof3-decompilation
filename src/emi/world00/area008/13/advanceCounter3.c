#include "internal.h"

/* @source 0x801F45E4
 * @behavior increments the area counter by 0x800
 */
void advanceCounter3(void) {
  s32 *counter;
  s32 value;

  counter = &counter3;
  value = *counter;
  value += 0x800;
  *counter = value;
}
