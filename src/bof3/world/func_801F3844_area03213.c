#include "bof3/world/area03213_internal.h"

/* @source 0x801F3844
 * @behavior Boots the scratch cursor record at 0x1F800044: submits the
 * positional effect through func_8019651C and stores the submitted effect byte
 * at 0x93 of the current work record at 0x80146884, sets record flag bit 0x20,
 * clears flag bit 0x40, sets the 0x5C byte and the -0x80 level triple at
 * 0x5D-0x5F, resets the 0x0C step to 0x2000, advances the 0x34 accumulator by
 * 0x2000, selects handler byte 1, and consumes two units of the countdown word
 * at 0x7E of the work record.
 * @status partial
 * @match 78.95
 * @residual allocator register assignment (45/57 instructions): the literal 1
 * shared by `D_1F800044[0x5c] = 1` and `cursor[4] = 1` is a single long-lived
 * value in the original that survives in $a0 from the 0x5C store through
 * `sb a0,4(v1)`, so $a0 is unavailable when the 0x34 accumulator is created and
 * the accumulator is assigned $v0; here it is assigned $a1, the accumulator
 * takes $a0, and the 0x34 load is then hoisted into the 0xC store's scheduler
 * slot with a wasted `nop` (57 vs 55 instructions, 228 vs 220 bytes). Register
 * probes that move the constant's last use ahead of the accumulator's (level
 * group order, named constant, declaration order) either reproduce this stream
 * unchanged (78.95%) or move the 0x5C constant into $a0 while stranding the
 * second cursor base in $a0 (77.19%); compiler-profile and permuter rungs are
 * not authorized for this lane and no clean-C shape measured so far reproduces
 * the original assignment. The per-region fresh cursor locals mirror the exact
 * area016 siblings func_801F3224/func_801F3360.
 */
void func_801F3844(void) {
  u8* record;
  u8* cursor;

  D_80146884[0x93] = func_8019651C(D_1F800044, 0, 0, 0, 1);
  D_1F800044[0] |= 0x20;
  D_1F800044[0] &= 0xbf;
  D_1F800044[0x5c] = 1;
  record = D_1F800044;
  *(s8*)(record + 0x5f) = -0x80;
  *(s8*)(record + 0x5e) = -0x80;
  *(s8*)(record + 0x5d) = -0x80;
  cursor = D_1F800044;
  *(s32*)(cursor + 0x0c) = 0x2000;
  *(s32*)(cursor + 0x34) += 0x2000;
  cursor[4] = 1;
  *(u16*)(D_80146884 + 0x7e) -= 2;
}
