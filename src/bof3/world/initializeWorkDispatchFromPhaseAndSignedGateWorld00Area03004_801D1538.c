#include "bof3/world/area03004_internal.h"

/* @behavior When shared phase byte 0x80145E92 is one, selects work mode zero.
 * Otherwise, when signed halfword 0x8014930E is below 56, selects work
 * mode one. Either selected path clears work step byte 3.
 * @source 0x801D1538
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkDispatchFromPhaseAndSignedGateWorld00Area03004(void) {
  if (D_80145E92 == 1u) {
    D_1F800044[2u] = 0u;
  } else if (D_8014930E < 56) {
    u8 nextWorkMode;
    do {
      nextWorkMode = 1u;
    } while (0);
    D_1F800044[2u] = nextWorkMode;
  } else {
    return;
  }
  D_1F800044[3u] = 0u;
}
