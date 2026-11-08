#include "bof3/bof3.h"

extern u16 D_80143F00;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 g_ScenarioProgress;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FE170
 * @behavior Overlay per-frame work-state handler, the word at 0x801FE958 that
 * is index 49 of the fifty-one-entry handler run at 0x801FE894, so the
 * work-byte-1 dispatcher reaches it with no arguments of its own and the two
 * packed record words the frame was entered with are still in $a0 and $a1.
 * It saves that pair in $s0 and $s1 before any call and never writes either.
 * The first arm requires the route word D_80143F00 to read 1, the first packed
 * word to equal the built literal 0x338000 and the second word high half to
 * fall in the inclusive range 0x27..0x28 (the `addiu -0x27` / `sltiu 2` pair).
 * Inside it, a clear shared flag 6 waits through func_8015B5D4(D_8014686C, 6)
 * runs the shared front-end startup helper func_8015C088, requests primary
 * scene state 4 with sub-state 5 by storing 4 in D_80146874 and 5 in
 * D_80146875, and returns 1; when flag 6 is set it instead waits through
 * func_8015B5D4(D_8014686C, 7) and, on a clear flag 7, runs the same startup
 * helper, clears the shared scenario progress byte g_ScenarioProgress
 * (0x80146864), requests primary scene state 5 with sub-state 0 and returns 1.
 * The second arm requires the route word to read 0x46, the first packed word
 * to equal the built literal 0x388000 and the second word high half to fall in
 * the inclusive range 0x2E..0x31 (the `addiu -0x2E` / `sltiu 4` pair), and it
 * sets flag bit 0x16 of the shared scenario flag word D_8014686C through
 * func_8015B580 without testing any flag first. Every arm is a branch to the
 * shared return path, so the body has one epilogue at 0x801FE288 that restores
 * the 0x20-byte frame; the zero of the trailing return is materialised in $v0
 * in the delay slot of each second-arm test and again after the flag call.
 *
 * Name evidence: the wording follows the accepted exact siblings of this
 * overlay, clearProgressRequestPrimaryState0AAndSetFlagBit13ScenarioScena0800_801FE0E0
 * (0x801FE0E0, same helper func_8015C088, same D_80146874 and D_80146875
 * request pair, same func_8015B580 flag-bit setter) and
 * clearProgressRequestPrimaryState7AndArmCountdownScenarioScena0800_801FE058
 * (0x801FE058), which call the same shared scenario-flag helpers; the route word
 * role of D_80143F00 and the packed-high-half range spelling are the accepted
 * exact forms of seedPrimaryStateOnRecordCallback (0x801FC818) and
 * requestPrimaryState5Substate10AndQueueFrontEndMode1FScenarioScena0300_801FCAB8
 * (0x801FCAB8). The stored 4, 5, 5, 0 and the flag ids 6, 7 and 0x16 are the
 * immediates in this span's own `addiu` and `jal` delay-slot words.
 *
 * Table evidence: recomputed from the payload, no image word of this target
 * holds 0x801FE170 except the one at 0x801FE958, so the run word is the only
 * reference and no jal targets it.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState4Or5OnClearedFlag6Or7AndSetFlagBit16OnRoute46PackedWordMatchScenarioScena0800_801FE170(
    s32 arg0, s32 arg1) {
  if (D_80143F00 == 1 && arg0 == 0x338000 && (u32)((arg1 >> 16) - 0x27) < 2u) {
    if (func_8015B5D4(D_8014686C, 6) == 0) {
      func_8015C088();
      D_80146874 = 4;
      D_80146875 = 5;
      return 1;
    }

    if (func_8015B5D4(D_8014686C, 7) == 0) {
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146874 = 5;
      D_80146875 = 0;
      return 1;
    }
  }

  if (D_80143F00 == 0x46 && arg0 == 0x388000 && (u32)((arg1 >> 16) - 0x2E) < 4u) {
    func_8015B580(D_8014686C, 0x16);
  }

  return 0;
}
