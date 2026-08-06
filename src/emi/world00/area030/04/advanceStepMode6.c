#include "internal.h"

/* @behavior increments scratch byte 0x03 when the shared state byte is 6.
 * @source 0x801D9E08
 */
void advanceStepMode6(void) {
  u8* scratch;

  if (modeByte == 6) {
    scratch = WORLD00_AREA030_SCRATCH_PTR;
    scratch[3]++;
  }
}
