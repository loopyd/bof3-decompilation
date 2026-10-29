#include "bof3/bof3.h"

extern s32 D_801468BC;

/* @source 0x801F3EE4
 * @behavior Increments the shared 32-bit work counter at 0x801468BC by 0x800
 *           in place and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounterWorld01Area06714_801F3EE4(void) {
  s32* counter;
  s32 value;

  counter = &D_801468BC;
  value = *counter;
  value += 0x800;
  *counter = value;
}
