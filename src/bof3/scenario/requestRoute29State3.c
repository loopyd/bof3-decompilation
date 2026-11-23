#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FC8AC
 * @behavior Route 0x29 handler of this overlay: the route dispatcher at
 * 0x801FC704 reaches it through its only reference, the jal at 0x801FC78C
 * taken when the shared route word D_80143F00 equals 0x29, and forwards both
 * incoming packed record words unchanged, so the body copies $a0 into $s1 and
 * $a1 into $s0 before any call and never writes either. It first queries the
 * shared scenario bank D_8014686C through func_8015B5D4(D_8014686C, 7) and
 * returns 0 when that reports nonzero. It then requires the second packed
 * word's high half to fall in 7..8 (sra 16 then addiu -7 / sltiu 2) and the
 * first word to equal 0x5C0000 (a plain lui, because that constant's low half
 * is zero), returning 0 when either test fails. On that match it sets flag 7
 * of the same bank through func_8015B580(D_8014686C, 7), requests primary
 * scene state 3 with sub-state 0xA by storing 3 in the shared state byte
 * D_80146874 and then 0xA in the shared sub-state byte D_80146875, and
 * returns 1: unlike the sibling route handlers it queues no front-end mode,
 * so its own result, not a tail callee's, is what the dispatcher returns.
 * The 0x98 bytes use the same 0x20-byte frame as those siblings and keep $s0,
 * $s1 and $ra across the two calls; the D_8014686C pointer load is scheduled
 * between the second saved-argument copy and the $ra store, the 0x5C0000
 * constant load fills the delay slot of the range test's branch, and the two
 * state-store constants share $v1 so the returned 1 can fill the final jump's
 * delay slot. No image word of this target holds 0x801FC8AC, so the
 * dispatcher's jal is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestRoute29State3(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 7) != 0) {
    return 0;
  }

  if ((u32)((arg1 >> 16) - 7) >= 2u) {
    return 0;
  }

  if (arg0 != 0x5C0000) {
    return 0;
  }

  func_8015B580(D_8014686C, 7);
  D_80146874 = 3;
  D_80146875 = 0xA;
  return 1;
}
