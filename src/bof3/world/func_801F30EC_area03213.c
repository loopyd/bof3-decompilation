#include "bof3/world/area03213_internal.h"

/* @source 0x801F30EC
 * @behavior Stores the slot identifier returned by func_8015CA48 at offset 3 of
 * the scratch cursor record at 0x1F800044 and, while that byte is not 0xFF,
 * marks the 0x98-byte work record of 0x80146888 it selects as live; the second
 * identifier from func_8015CA48 goes to cursor offset 4 and, while it is not
 * 0xFF, activates both selected work records through func_801A4D54 with the
 * local 0x801F3F8C data script, restores the previously published cursor,
 * spreads the two records' 0x34/0x38 accumulators by -/+0x1000, sets their 0x20
 * flag bit, clears their 0x5C byte and submits both through func_8019651C with
 * the 0xF mask; a 0xFF identifier instead releases the record of the earlier
 * slot and returns, and the full path ends by setting cursor byte 9 to 4 and
 * cursor byte 1 to 1.
 * @status partial
 * @match 69.13
 * @residual same-size allocator/scheduler residual (159/230 instructions, 920
 * vs 916 bytes, first difference +0x0000 in the frame layout): the slot offset
 * is computed in $v1 and the loaded accumulator in $v0, where the original
 * keeps the offset in $v0 and loads into $v1, and the frame is 0x30 instead of
 * 0x38 (two fewer local words). Measured clean-C shapes: inlining the multiply
 * (`*(s32*)(D_80146888 + cursor[3] * 0x98 + 0x34) -= 0x1000;`) reproduces the
 * offset register but makes cse keep the 0x80146888 base in a callee-saved
 * register and turn every field access into base+displacement (127/229,
 * 848 bytes); the struct form `((World00Area032WorkRecord*)D_80146888)[i].f`
 * behaves the same way (155/229, 212 instructions); the explicit `offset` local
 * used here restores the folded `%hi/%lo(at)` addressing but shifts the offset
 * register, and the same $v1/$v0 class is already recorded for the sibling
 * partial func_801F3480. Permuter, flag-search and compiler-variant rungs are
 * not authorized for this lane, so the residual is left for the next owner.
 */
void func_801F30EC(void) {
  u32 offset;
  u8* cursor;
  u8* pair;
  u16* slot;

  D_1F800044[3] = func_8015CA48();
  if (D_1F800044[3] == 0xFF) {
    return;
  }

  offset = D_1F800044[3] * 0x98;
  D_80146888[offset] = 1;
  D_1F800044[4] = func_8015CA48();
  if (D_1F800044[4] == 0xFF) {
    offset = D_1F800044[3] * 0x98;
    D_80146888[offset] = 0;
    return;
  }

  offset = D_1F800044[4] * 0x98;
  D_80146888[offset] = 1;
  cursor = D_1F800044;
  slot = &D_1F800000;

  *slot = cursor[3];
  func_801A4D54(&D_801F3F8C);
  *slot = cursor[4];
  func_801A4D54(&D_801F3F8C);
  D_1F800044 = cursor;

  offset = cursor[3] * 0x98;
  *(s32*)(D_80146888 + offset + 0x34) -= 0x1000;
  offset = cursor[3] * 0x98;
  *(s32*)(D_80146888 + offset + 0x38) += 0x1000;
  offset = cursor[4] * 0x98;
  *(s32*)(D_80146888 + offset + 0x34) += 0x1000;
  offset = cursor[4] * 0x98;
  *(s32*)(D_80146888 + offset + 0x38) -= 0x1000;
  offset = cursor[3] * 0x98;
  D_80146888[offset] |= 0x20;
  offset = D_1F800044[4] * 0x98;
  D_80146888[offset] |= 0x20;
  pair = D_1F800044;
  offset = pair[4] * 0x98;
  D_80146888[offset + 0x5C] = 0;
  offset = pair[3] * 0x98;
  D_80146888[offset + 0x5C] = 0;

  func_8019651C(D_80146888 + cursor[3] * 0x98, 0xF, 0, 0, 1);
  func_8019651C(D_80146888 + cursor[4] * 0x98, 0, 0, 0xF, 1);

  D_1F800044[9] = 4;
  D_1F800044[1] = 1;
}
