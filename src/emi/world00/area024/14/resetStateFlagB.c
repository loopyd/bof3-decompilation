#include "internal.h"

/* @behavior slot-1 callback of the local pointer table D_801F5AB4:
 * sets D_801490A8 to -1.
 * @source 0x801F41EC
 */
void resetStateFlagB(void) {
  D_801490A8 = 0xFFFF;
}
