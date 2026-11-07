#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(s32 arg0);

/* @source 0x801FCAB8
 * @behavior Route 0x32 handler of this overlay: the dispatcher at 0x801FC704
 * reaches it through the jal at 0x801FC79C, forwarding both incoming packed
 * record words unchanged, so the body saves $a0 in $s0 and $a1 in $s1 before
 * any call and never writes either. It first waits through the shared
 * two-stage helper func_8015B5D4(D_8014686C, 0x1A); when that reports zero it
 * returns 0, then requires the first word's high half to read 0x15 and the
 * second word's high half to fall in the inclusive range 0x34..0x35 (the
 * `addiu -0x34` / `sltiu 2` pair), and returns 0 when either test fails.
 * On that path it requests primary scene state 5 with sub-state 0x10 by
 * storing 5 in the shared state byte D_80146874 and then 0x10 in the shared
 * sub-state byte D_80146875, queues front-end mode 0x1F through the overlay
 * tail at 0x801FCC9C and returns that tail's result sign-extended from its low
 * byte (sll/sra 24); the tail always answers 1. The 0x94 bytes use a 0x20-byte
 * frame that keeps $s0, $s1 and $ra across the two calls, the D_8014686C
 * pointer load is scheduled between the second saved-argument copy and the $ra
 * store, and the constant 5 of the first state store fills the delay slot of
 * the range test's branch. No image word of this target holds 0x801FCAB8, so
 * the dispatcher's jal is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState5Substate10AndQueueFrontEndMode1FScenarioScena0300_801FCAB8(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 0x1A) == 0) {
    return 0;
  }

  if ((arg0 >> 16) != 0x15) {
    return 0;
  }

  if ((u32)((arg1 >> 16) - 0x34) >= 2u) {
    return 0;
  }

  D_80146874 = 5;
  D_80146875 = 0x10;
  return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(0x1F);
}
