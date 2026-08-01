#include "internal.h"

/* @source 0x801E1B80
 * @behavior jump-table dispatcher: calls D_801E5D2C[arg0], passing the
 *           zero-extended command id scaled by 4 as the handler argument.
 */
void func_801E1B80(u8 arg0) {
  u32 index = (u32)arg0 << 2;

  D_801E5D2C[arg0](index);
}
