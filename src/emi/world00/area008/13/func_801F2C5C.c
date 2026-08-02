#include "internal.h"

/* @behavior Clears scratch byte 9 and local state, then enables scratch byte 1.
 * @source 0x801F2C5C
 */
void func_801F2C5C(void) {
  D_1F800044->unk_09 = 0u;
  D_801F53F4 = 0u;
  D_1F800044->mode = 1u;
}
