#include "internal.h"

/* @source 0x801F4600
 * @behavior decrements the area counter by 0x800
 */
void func_801F4600(void) {
  s32 *counter;
  s32 value;

  counter = &D_80146C4C;
  value = *counter;
  value -= 0x800;
  *counter = value;
}
