#include "internal.h"

/* @behavior increments scratch byte 0x03 when the shared state byte is 6.
 * @source 0x801D9E08
 */
void func_801D9E08(void) {
  u8* scratch;

  if (D_80144125 == 6) {
    scratch = WORLD00_AREA030_SCRATCH_PTR;
    scratch[3]++;
  }
}
