#include "internal.h"

/* @behavior dispatches the indexed handler at D_801CD49C when enabled by
 * D_80146260.
 * @source 0x801C0A68
 */
void func_801C0A68(void) {
  if (D_80146260 != 0) {
    D_801CD49C[D_80146261]();
  }
}
