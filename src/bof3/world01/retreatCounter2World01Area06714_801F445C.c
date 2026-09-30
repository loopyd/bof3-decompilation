#include "bof3/bof3.h"

extern s32 D_80146954;

/* @source 0x801F445C
 * @behavior Decrements the shared 32-bit work counter at 0x80146954 by 0x800
 *           in place and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2World01Area06714_801F445C(void) {
  s32* counter;
  s32 value;

  counter = &D_80146954;
  value = *counter;
  value -= 0x800;
  *counter = value;
}
