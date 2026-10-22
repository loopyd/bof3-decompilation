#include "bof3/world/area03213_internal.h"

/* @source 0x801F30EC
 * @behavior Stores the slot identifier returned by func_8015CA48 at offset 3 of
 * the scratch cursor record published at 0x1F800044 and returns when that byte
 * is 0xFF. Otherwise it marks the 0x98-byte work record of 0x80146888 that the
 * byte selects as live, asks func_8015CA48 for a second identifier and stores it
 * at cursor offset 4; when that second byte is 0xFF the first record is released
 * and the function returns. Otherwise it marks the second record live, activates
 * both records through func_801A4D54 with the local 0x801F3F8C data script while
 * publishing the selected slot index at 0x1F800000, restores the previously
 * published cursor, spreads the two records' 0x34/0x38 accumulators by
 * -0x1000/+0x1000, sets their 0x20 flag bit, clears their 0x5C byte, submits both
 * through func_8019651C with the 0xF mask, and finally sets cursor byte 9 to 4
 * and cursor byte 1 to 1.
 * @status partial
 * @match 91.30
 * @residual frame and scheduler residual (210/229 instructions, 920 vs 916
 * bytes). The measured clean-C shape is the split read-modify-write: load the
 * record byte or accumulator into a local, apply the operation as its own
 * statement, then store, which reproduces the original's $v0 scaled-offset /
 * $v1 value registers at all six sites exactly as the exact sibling
 * func_801F3480 records it. The remaining difference is the frame: the original
 * reserves 0x38 bytes of frame with 32 bytes below the save area, this build 0x30
 * with 24, and no original instruction addresses the extra eight bytes (the only
 * stack references are the five saves, the two 0x10 outgoing argument stores and
 * the epilogue), so the extra local area is an allocation artifact that no
 * evidenced source shape reproduces; the prologue/epilogue offsets and the
 * instruction placement of the func_8019651C argument setup (0xF/0/0 argument
 * constants and the hoisted cursor[3] load) follow from it. Measured and rejected
 * this lane: one folded `offset` local per site (159/229, 920 bytes), struct
 * member indexing through World00Area032WorkRecord (160/229, 840 bytes), the 0xF
 * mask carried in a local (210/229, unchanged), and a local 5th argument
 * (210/229, unchanged). Compiler-profile, flag-search and permuter rungs are not
 * authorized for this lane and remain untried.
 */
void func_801F30EC(void) {
  u32 offset;
  s32 accum;
  u8 value;
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
  accum = *(s32*)(D_80146888 + offset + 0x34);
  accum -= 0x1000;
  *(s32*)(D_80146888 + offset + 0x34) = accum;
  offset = cursor[3] * 0x98;
  accum = *(s32*)(D_80146888 + offset + 0x38);
  accum += 0x1000;
  *(s32*)(D_80146888 + offset + 0x38) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32*)(D_80146888 + offset + 0x34);
  accum += 0x1000;
  *(s32*)(D_80146888 + offset + 0x34) = accum;
  offset = cursor[4] * 0x98;
  accum = *(s32*)(D_80146888 + offset + 0x38);
  accum -= 0x1000;
  *(s32*)(D_80146888 + offset + 0x38) = accum;

  offset = cursor[3] * 0x98;
  value = D_80146888[offset];
  value |= 0x20;
  D_80146888[offset] = value;
  offset = D_1F800044[4] * 0x98;
  value = D_80146888[offset];
  value |= 0x20;
  D_80146888[offset] = value;

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
