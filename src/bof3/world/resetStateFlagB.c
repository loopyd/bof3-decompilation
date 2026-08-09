#include "bof3/world/area02414_internal.h"

/* @behavior slot-1 callback of the local pointer table D_801F5AB4:
 * sets D_801490A8 to -1.
 * @source 0x801F41EC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetStateFlagB(void) {
  D_801490A8 = 0xFFFF;
}
