#include "internal.h"

/* @behavior clears the scratch-record flag when the AREA030 state byte is >= 10.
 * @source 0x801E023C
 */
void world00_area030_flag_clear_state10(void) {
  if (D_80145E93 >= 10u) {
    *WORLD00_AREA030_SCRATCH_PTR = 0u;
  }
}
