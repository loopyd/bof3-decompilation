#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u16 D_80145AA8;
extern u16 D_80145AB8;
extern u8 D_8014623B;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end request setter at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801FA08C
 * @behavior Word 7 of the seventeen-word in-image primary state table at
 * 0x801FE3EC, the word stored at 0x801FE408, that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801FA08C and that one table word is
 * its only reference in this payload. It reads the shared sub-state byte
 * D_80146875 once through one address taken into a local pointer, which keeps
 * that base in $s0 for the whole body, and it runs two alternative sequences
 * over that byte. Sub-state 0, while the unsigned shared mode byte D_8014623B
 * (0x8014623B, read with lbu) is not 2, runs the shared helper
 * func_801BE1B0(0) and advances the sub-state byte to 1. Sub-state 1, once the
 * front-end phase byte D_80143BB0 (0x80143BB0, read with lbu) leaves 2, sets
 * scenario flag bit 0x13 of the shared flag word D_8014686C (0x8014686C, read
 * with lw) through func_8015B580 and falls into the shared clearing tail.
 * Sub-state 5, while the halfword flag pair D_80145AA8 and D_80145AB8
 * (0x80145AA8 and 0x80145AB8, both read with lhu) has no bit in common and the
 * front-end phase byte reads 0, clears the shared scenario progress byte
 * g_ScenarioProgress (0x80146864), advances the sub-state byte to 6, and only
 * then seeds the front selector context func_8019FA28(0x76, 0x490000, 0x188000,
 * 0x81), whose own tail arms that phase byte to 5. Sub-state 6, once the
 * progress byte reads 1 and the phase byte leaves 2 again, clears the progress
 * byte and falls into that same clearing tail, which runs the shared release
 * helper func_8015C058 and clears the sub-state byte and the primary state byte
 * D_80146874 back to zero, ending the state. Every other sub-state value returns
 * without touching the tail. It takes no arguments, reads no other state and
 * writes no other byte, and its 0x18-byte frame only keeps $ra and $s0, while
 * the constant 1 of the sub-state compare stays live in $a1 for the sub-state 6
 * progress compare and the constant 2 of the sub-state 0 mode test is
 * materialised in $v0 before the branch into that case. Its 0x164 bytes have no
 * copy in the tree: the 0x164-byte span itself occurs only at its own offset,
 * and the closest unrelated 89-word window in any other payload shares 18 of
 * its words, while the three other D_8014623B readers (scena13 0x801FB1B4,
 * scena12 0x801FB864 and this payload's own 0x801FA8B8) differ in 87, 84 and 88
 * of 89 words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armProgress0UnlessMode2ThenSetFlagBit13OrSeedFrontSelectorContext76AndClearStateScenarioScena0900_801FA08C(void) {
  u8 *state = &D_80146875;

  switch (*state) {
  case 0:
    if (D_8014623B == 2u) {
      return;
    }
    func_801BE1B0(0u);
    *state = 1u;
    return;
  case 1:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015B580(D_8014686C, 0x13);
    break;
  case 5:
    if ((D_80145AA8 & D_80145AB8) != 0u) {
      return;
    }
    if (D_80143BB0 != 0u) {
      return;
    }
    g_ScenarioProgress = 0u;
    *state = 6u;
    func_8019FA28(0x76, 0x490000, 0x188000, 0x81);
    return;
  case 6: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1u) {
      return;
    }
    if (D_80143BB0 == 2u) {
      return;
    }
    *progress = 0u;
    break;
  }
  default:
    return;
  }

  func_8015C058();
  *state = 0u;
  D_80146874 = 0u;
}
