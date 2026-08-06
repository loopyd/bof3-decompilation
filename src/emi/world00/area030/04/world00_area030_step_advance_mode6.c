#include "internal.h"

/* @behavior increments scratch byte 0x03 when the shared state byte is 6.
 * @source 0x801D9E08
 */
void world00_area030_step_advance_mode6(void) {
  u8* scratch;

  if (world00_area030_modeByte == 6) {
    scratch = WORLD00_AREA030_SCRATCH_PTR;
    scratch[3]++;
  }
}
