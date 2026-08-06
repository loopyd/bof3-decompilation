#include "internal.h"

/* @behavior Clears scratch byte 9 and local state, then enables scratch byte 1.
 * @source 0x801F2C5C
 */
void resetStateSelectMode1(void) {
  g_areaWork->unk_09 = 0u;
  countdown = 0u;
  g_areaWork->mode = 1u;
}
