#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F78CC
 * @behavior Takes no arguments and returns nothing: resets the shared
 * scratchpad work-flag block through the shared func_80196070 helper, which
 * zeroes work-area bytes 0x00-0x04; no other state is read or written. The
 * address is entry 10 of the per-frame handler table at 0x801FC980 indexed by
 * byte 0x01 of the scratchpad work object published at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F78CC(void) {
  func_80196070();
}
