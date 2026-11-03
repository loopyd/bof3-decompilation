#include "bof3/world/area03004_internal.h"

/* @behavior arms the AREA030 work-record state byte 2 with zero while the shared
 * phase byte 0x80145E92 reads one, and with one while the shared halfword
 * 0x8014930E is still below 56; either armed path clears the stage byte 3.
 * @source 0x801D1538
 * @status partial
 * @match 84.00
 * @residual 21 of 25 current instructions match 25 original at the same 100-byte
 * size. The compares, both stores and the merge agree. The store value takes the
 * load-delay slot in this build, where the original places it in the branch-delay
 * slot of the 0x8014930E test, so the pointer and value registers swap with it.
 */
void func_801D1538(void) {
  if (D_80145E92 == 1u) {
    D_1F800044[2u] = 0u;
  } else if (D_8014930E < 56) {
    D_1F800044[2u] = 1u;
  } else {
    return;
  }
  D_1F800044[3u] = 0u;
}
