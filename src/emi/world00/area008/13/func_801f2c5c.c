#include "internal.h"

/* @behavior Clears scratch byte 9 and local state, then enables scratch byte 1.
 * @source 0x801f2c5c func_801f2c5c
 */
void func_801f2c5c(void) {
  u8* scratch;

  scratch = *(u8**)0x1f800044u;
  scratch[9] = 0u;
  scratch = *(u8**)0x1f800044u;
  REG8(0x801f53f4u) = 0u;
  scratch[1] = 1u;
}
