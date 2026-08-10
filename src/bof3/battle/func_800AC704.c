#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC704
 * @behavior Find the first of eight records whose leading byte matches id.
 * @status partial
 * @match 95.00
 * @residual One same-size operand-order mismatch at +0x1C: original
 * addu at,v0,at versus current addu at,at,v0; canonical and installed
 * historical compiler profiles produced no exact result; permuter cannot
 * resolve the semantic source path to its target-local original assembly.
 */
u8 func_800AC704(u8 id)
{
  u8 i;

  i = 0;
  while (i < 8) {
    if (D_800E4050[i].id == id) {
      return i;
    }
    i++;
  }
  return 0xFF;
}
