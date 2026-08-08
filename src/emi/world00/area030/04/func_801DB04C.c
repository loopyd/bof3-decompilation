#include "internal.h"

/**
 * @source 0x801DB04C
 * @behavior Selects a signed record value and records which slot was used.
 */
void func_801DB04C(void) {
  if (D_1F800044[9] != 0) {
    D_801E31F8 = 2;
    D_8014421C = D_801E320C[7];
  } else {
    D_801E31F8 = 1;
    D_8014421C = D_801E320C[4];
  }
}
