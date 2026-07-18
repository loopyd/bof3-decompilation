#include "internal.h"

/* @behavior Clears scratch byte 9 and local state, then enables scratch byte 1.
 * @source 0x801F2C5C
 */
void func_801F2C5C(void) {
  u8* scratch;

  scratch = *(u8**)0x1f800044u;
  scratch[9] = 0u;
  scratch = *(u8**)0x1f800044u;
  MMIO8(0x801f53f4u) = 0u;
  scratch[1] = 1u;
}
