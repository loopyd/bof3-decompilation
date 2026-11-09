#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;

/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end request setter at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F841C
 * @behavior Word 1 (the word stored at 0x801FE3F0) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets this address and that table word is its
 * only reference. It reads the shared sub-state byte D_80146875 once through one
 * address taken into a local pointer, which keeps that base in $s0 across both
 * of its calls: sub-state 0 advances the sub-state byte to 1, sets scenario flag
 * bit 0 of the shared flag word D_8014686C (0x8014686C, read with lw) through
 * func_8015B580 and then requests front-selector context seed 0x45 with the two
 * 16.16 context words 0x1F8000 and 0x4C0000 and kind 0x86 through
 * func_8019FA28, whose phase byte D_80143BB0 is what makes the request visible;
 * sub-state 1 arms the effect bank D_8014832E = 0x1F and advances the sub-state
 * byte to 2; sub-state 2, once the shared scenario progress byte
 * g_ScenarioProgress (0x80146864) reads 0x11, sets scenario flag bit 1 of the
 * same flag word through func_8015B580, runs the shared overlay release helper
 * func_8015C058 and then clears the sub-state byte and the primary state byte
 * D_80146874 back to zero, which ends the state; every other sub-state value
 * does nothing. It takes no arguments, reads no other state and writes no other
 * byte, and its 0x18-byte frame only keeps $ra and $s0. Its 0xE8 bytes keep the
 * sub-state-compare shape of the exact sibling
 * waitProgress3ThenSetFlagBit24AndRequestPrimaryState0ScenarioScena0900_801FBC78
 * (0x801FBC78), which switches on the same byte and gates a case on the same
 * progress byte: that sibling's case-1 test also materialises its progress
 * constant in the branch delay slot and takes the same two-call, clear-both-
 * bytes tail, so this file copies that switch shape and writes its own
 * constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFlagBit0RequestFrontSelectorContextArmEffectBankThenSetFlagBit1AndClearStateOnProgress11ScenarioScena0900_801F841C(void) {
  u8 *state = &D_80146875;

  switch (*state) {
  case 0:
    *state = 1u;
    func_8015B580(D_8014686C, 0);
    func_8019FA28(0x45, 0x1F8000, 0x4C0000, 0x86);
    break;
  case 1:
    D_8014832E = 0x1F;
    *state = 2u;
    break;
  case 2:
    if (g_ScenarioProgress != 0x11u) {
      break;
    }
    func_8015B580(D_8014686C, 1);
    func_8015C058();
    *state = 0u;
    D_80146874 = 0u;
    break;
  }
}
