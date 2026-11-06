#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801FBC78
 * @behavior Word 16 (the word stored at 0x801FE42C) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets this address and that table word is its
 * only reference. It reads the shared sub-state byte D_80146875 once through one
 * address taken into a local pointer, which keeps that base in $s0 across both
 * calls: sub-state 0 runs the shared helper func_801BE1B0(2) and advances the
 * sub-state byte to 1; sub-state 1, once the shared scenario progress byte
 * g_ScenarioProgress (0x80146864) reads 3, sets scenario flag bit 0x24 of the
 * shared flag word D_8014686C (0x8014686C, read with lw) through func_8015B580,
 * runs the shared release helper func_8015C058 and then clears the sub-state
 * byte and the primary state byte D_80146874 back to zero, which ends the state;
 * every other sub-state value does nothing. It takes no arguments, reads no
 * other state and writes no other byte, and its 0x18-byte frame only keeps $ra
 * and $s0. Its 0x94 bytes keep the sub-state-compare shape of the scena00
 * controller func_801FB204, which switches on the same byte and gates a case on
 * the same progress byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitProgress3ThenSetFlagBit24AndRequestPrimaryState0ScenarioScena0900_801FBC78(void) {
  u8 *state = &D_80146875;

  switch (*state) {
  case 0:
    func_801BE1B0(2u);
    *state = 1u;
    break;
  case 1:
    if (g_ScenarioProgress != 3u) {
      break;
    }
    func_8015B580(D_8014686C, 0x24);
    func_8015C058();
    *state = 0u;
    D_80146874 = 0u;
    break;
  }
}
