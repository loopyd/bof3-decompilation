#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DB04C
 * @behavior Selects a signed record value and records which slot was used.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void selectRecordValueAndSlot(void) {
  if (D_1F800044[9] != 0) {
    D_801E31F8 = 2;
    D_8014421C = D_801E320C[7];
  } else {
    D_801E31F8 = 1;
    D_8014421C = D_801E320C[4];
  }
}
