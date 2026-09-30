#include "bof3/bof3.h"

extern s32 D_80146954;

/* @source 0x801F4440
 * @behavior Increments the shared 32-bit work counter at 0x80146954 by 0x800
 *           in place and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2World01Area06714_801F4440(void) {
  s32* counter;
  s32 value;

  counter = &D_80146954;
  value = *counter;
  value += 0x800;
  *counter = value;
}
