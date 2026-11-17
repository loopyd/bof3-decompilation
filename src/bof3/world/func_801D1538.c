#include "bof3/world/area03004_internal.h"

/* @behavior arms the AREA030 work-record state byte 2 with zero while the shared
 * phase byte 0x80145E92 reads one, and with one while the shared halfword
 * 0x8014930E is still below 56; either armed path clears the stage byte 3.
 * @source 0x801D1538
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1538(void) {
  if (D_80145E92 == 1u) {
    D_1F800044[2u] = 0u;
  } else if (D_8014930E < 56) {
    u8 state;
    do {
      state = 1u;
    } while (0);
    D_1F800044[2u] = state;
  } else {
    return;
  }
  D_1F800044[3u] = 0u;
}
