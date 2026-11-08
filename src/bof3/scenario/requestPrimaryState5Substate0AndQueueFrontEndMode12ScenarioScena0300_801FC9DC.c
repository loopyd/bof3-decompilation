#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(s32 arg0);

/* @source 0x801FC9DC
 * @behavior Route 0x45 handler of this overlay: the dispatcher at 0x801FC704
 * reaches it through the jal at 0x801FC7BC and forwards both incoming packed
 * record words unchanged, so the body saves
 * $a0 in $s1 and $a1 in $s0 before any call and never writes either. It first
 * waits through the shared two-stage helper func_8015B5D4(D_8014686C, 0x18);
 * when that reports nonzero it returns 0. It then accepts three alternative
 * packed-record pairs, each tested as an equality against one word plus the
 * `addiu`/`sltiu 2` high-half range of the other, and returns 0 when all three
 * fail: the first word equal to 0x3A8000 with the second word's high half in
 * 0x3C..0x3D, the second word equal to 0x28000 with the first word's high
 * half in 0x1F..0x20, and the second word equal to 0x4C8000 with the same
 * first-word range. On that path it requests primary scene state 5 with
 * sub-state 0 by storing 5 in the shared state byte D_80146874 and then 0 in
 * the shared sub-state byte D_80146875, queues front-end mode 0x12 through the
 * overlay tail at 0x801FCC9C and returns that tail's result sign-extended from
 * its low byte (sll/sra 24).
 * The 0xDC bytes use a 0x20-byte frame that keeps $s0, $s1 and $ra across the
 * two calls: the D_8014686C pointer load is scheduled between the second
 * saved-argument copy and the $ra store, and the constant 5 of the first state
 * store fills the delay slot of each range test's branch. The three tested
 * words are built as lui/ori pairs, so they are literal constants and not
 * table entries. No image word of this target holds 0x801FC9DC, so the
 * dispatcher's jal is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState5Substate0AndQueueFrontEndMode12ScenarioScena0300_801FC9DC(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 0x18) != 0) {
    return 0;
  }

  if ((arg0 == 0x3A8000 && (u32)((arg1 >> 16) - 0x3C) < 2u) ||
      (arg1 == 0x28000 && (u32)((arg0 >> 16) - 0x1F) < 2u) ||
      (arg1 == 0x4C8000 && (u32)((arg0 >> 16) - 0x1F) < 2u)) {
    D_80146874 = 5;
    D_80146875 = 0;
    return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(0x12);
  }

  return 0;
}
