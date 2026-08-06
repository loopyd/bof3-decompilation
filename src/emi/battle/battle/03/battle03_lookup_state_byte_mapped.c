#include "internal.h"

/* @source 0x801DD7AC
 * @behavior returns one indexed state byte, mapping value seven to zero.
 */
u8 battle03_lookup_state_byte_mapped(s32 arg0) {
  u8 var = D_80181B10[arg0 & 0xFF];

  if ((var & 0xFF) == 7) {
    var = 0;
  }
  return var;
}
