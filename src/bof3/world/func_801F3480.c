#include "bof3/world/area03213_internal.h"

/* @behavior updates work-area activity flags from the scratch cursor record.
 * @source 0x801F3480
 * @status exact
 * @match 100.00
 * @residual none
 * Clean-C shape: each flag update loads the record byte into `value`, applies
 * the bit operation to `value` as its own statement, and only then stores it.
 * Folding the operation into the store (`= value | 0x40`) keeps the scaled slot
 * offset in $v1 instead of the original $v0 and costs four instructions per
 * site; the split statement reproduces all 77 instructions and 308 bytes.
 */
void func_801F3480(void) {
  s32 offset;
  u8 value;

  if (D_1F800044[9] != 0) {
    offset = D_1F800044[11] * 152;
    value = D_80146888[offset];
    value |= 0x40;
    D_80146888[offset] = value;
    D_1F800044[9]--;
  } else {
    offset = D_1F800044[11] * 152;
    value = D_80146888[offset];
    value &= 0xBF;
    D_80146888[offset] = value;
    offset = D_1F800044[3] * 152;
    D_80146888[offset] = 0;
    offset = D_1F800044[4] * 152;
    D_80146888[offset] = 0;
    D_1F800044[1] = 2;
  }
}
