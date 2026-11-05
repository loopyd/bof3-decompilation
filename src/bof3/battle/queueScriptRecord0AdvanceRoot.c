#include "bof3/battle/battle15_internal.h"

/* @source 0x80096AB0
 * @behavior Calls func_801DE94C(0, 0), then increments the byte at 0x801462E3 modulo
 * 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueScriptRecord0AdvanceRoot(void) {
  func_801DE94C(0, 0);
  PSX_REF(u8, (u32)&D_801462E3) += 1;
}
