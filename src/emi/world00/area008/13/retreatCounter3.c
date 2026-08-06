#include "internal.h"

/* @source 0x801F4600
 * @behavior decrements the area counter by 0x800
 */
void retreatCounter3(void) {
  s32 *counter;
  s32 value;

  counter = &counter3;
  value = *counter;
  value -= 0x800;
  *counter = value;
}
