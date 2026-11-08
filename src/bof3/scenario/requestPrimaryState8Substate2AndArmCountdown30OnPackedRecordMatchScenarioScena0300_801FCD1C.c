#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FCD1C
 * @behavior Route-0x47 packed-record handler of this overlay: the route-0x47
 * gate at 0x801FCCDC reaches it through its jal at 0x801FCCFC and forwards
 * both incoming packed record words unchanged, so the body saves $a0 in $s1
 * and $a1 in $s0 before any call and never writes either. It first waits
 * through the shared two-stage helper func_8015B5D4(D_8014686C, 0x1E) and
 * returns 0 when that reports nonzero, then requires the second word to equal
 * 0x490000 and the first word's high half to fall in the inclusive range
 * 0x0E..0x11 (the `addiu -0xE` / `sltiu 4` pair), returning 0 when either test
 * fails. On that path it calls the shared no-argument helper func_8015C088,
 * requests primary scene state 8 with sub-state 2 by storing 8 in the shared
 * state byte D_80146874 and then 2 in the shared sub-state byte D_80146875,
 * arms the shared frame countdown D_80146876 with 0x1E and answers 1. The
 * 0x98 bytes use a 0x20-byte frame that keeps $s0, $s1 and $ra across the two
 * calls, with the D_8014686C pointer load scheduled between the second
 * saved-argument copy and the $ra store, and every return-0 path materialises
 * its zero as `addu $v0,$zero,$zero` in the branch delay slot. No image word
 * of this target holds 0x801FCD1C, so the gate's jal is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState8Substate2AndArmCountdown30OnPackedRecordMatchScenarioScena0300_801FCD1C(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 0x1E) != 0) {
    return 0;
  }

  if (arg1 != 0x490000) {
    return 0;
  }

  if ((u32)((arg0 >> 16) - 0xE) >= 4u) {
    return 0;
  }

  func_8015C088();
  D_80146874 = 8;
  D_80146875 = 2;
  D_80146876 = 0x1E;
  return 1;
}
