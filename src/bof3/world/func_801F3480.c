#include "bof3/world/area03213_internal.h"

/* @behavior updates work-area activity flags from the scratch cursor record.
 * @source 0x801F3480
 * @status partial
 * @match 79.22
 * @residual same-size allocator mismatch begins at +0x34: computed slot
 * offset is in v1 instead of v0; one bounded permuter run found no exact. A
 * shape sweep of the inline compound-assignment form and of the struct-field
 * form `((World00Area032WorkRecord*)D_80146888)[i].flags_00` measured 55/77 at
 * 284 bytes, six instructions short of the original, so this offset-local form
 * remains the best coherent candidate.
 */
void func_801F3480(void) {
  s32 offset;
  u8 value;

  if (D_1F800044[9] != 0) {
    offset = D_1F800044[11] * 152;
    value = D_80146888[offset];
    D_80146888[offset] = value | 0x40;
    D_1F800044[9]--;
  } else {
    offset = D_1F800044[11] * 152;
    value = D_80146888[offset];
    D_80146888[offset] = value & 0xBF;
    offset = D_1F800044[3] * 152;
    D_80146888[offset] = 0;
    offset = D_1F800044[4] * 152;
    D_80146888[offset] = 0;
    D_1F800044[1] = 2;
  }
}
