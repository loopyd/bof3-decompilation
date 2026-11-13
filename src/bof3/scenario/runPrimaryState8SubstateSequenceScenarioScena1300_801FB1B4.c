#include "bof3/bof3.h"

extern u8 D_80143BB0;
extern u8 D_801460FB;
extern u8 D_8014623B;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 g_ScenarioProgress;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared front-end action queue helper at 0x80150224. */
void func_80150224(u32 action);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 flags, s32 bit);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared main-executable helper at 0x801A78F8; role not established. */
void func_801A78F8(void);

/* @source 0x801FB1B4
 * @behavior Entry 8, the last pointer word of the nine-entry handler table at
 * 0x801FBF54 that the exact dispatcher
 * dispatchPrimaryStateHandlerScenarioScena1300_801F8474 (0x801F8474) selects
 * with the shared primary state byte D_80146874 and calls with no arguments, so
 * no in-image jal targets 0x801FB1B4 and that one table word is its only
 * reference: the sub-state switch of the overlay's primary state 8 handler. It
 * reads the shared sub-state byte D_80146875 with lbu, bounds it against 0xC
 * with sltiu and dispatches through the twelve-word absolute table at
 * 0x801F6E74 in this payload's leading table data, whose entries for sub-states
 * 1, 2, 3, 7 and 9 point at the body exit, so those five values are the default
 * and the body has cases 0, 4, 5, 6, 8, 0x0A and 0x0B.
 *
 * Sub-state 0, once both shared gate bytes D_801460FB (0x801460FB) and
 * D_8014623B (0x8014623B) read zero, runs func_801BE1B0 with 0x0A, sets bit
 * 0x1D of the shared flag word D_8014686C (0x8014686C, read with lw) through
 * func_8015B580, and then clears the primary state byte D_80146874 and the
 * sub-state byte. Sub-state 4 queues front-end action 1 through func_80150224,
 * stores 2 in the shared front-end phase byte D_80143BB0 (0x80143BB0) and
 * advances the sub-state byte to 6; sub-state 5 repeats that tail after queuing
 * action 0x4E, so the two cases share their last two stores and case 4 jumps
 * over the action immediate of case 5 into the common tail. Sub-state 6, while
 * the phase byte leaves 2, runs the shared release helper func_8015C058 and
 * falls into the same clearing tail of D_80146874 and D_80146875 that sub-state
 * 0 ends on. Sub-state 8, once the shared progress byte g_ScenarioProgress
 * (0x80146864) reads 0x28, clears it and advances the sub-state byte to 0x0A.
 * Sub-state 0x0A, while the phase byte leaves 2, seeds the front-selector
 * context func_8019FA28(0x97, 0x360000, 0x1D0000, 3) and advances the sub-state
 * byte to 0x0B. Sub-state 0x0B, while the phase byte reads 0, sets bit 0x1E of
 * D_8014686C through func_8015B580, runs the release helper func_8015C058,
 * clears D_80146874 and D_80146875 and ends on func_801A78F8.
 *
 * The body reads no other state, writes no other byte and takes no arguments;
 * its 0x18-byte frame only keeps $ra across the calls, and the sub-state read is
 * hoisted above that frame's allocation, which is why the original bytes open
 * with the `lui`/`lbu` pair on D_80146875 before the `addiu $sp`/`sw $ra`
 * prologue. Sub-state 8 takes the address of g_ScenarioProgress into a local
 * pointer, which is why its `lui`/`addiu` base reaches both the `lbu` and the
 * clearing `sb` through one register; writing the two accesses as bare symbol
 * references instead folds the load and re-materialises `$at` for the store,
 * and that spelling measured 99/103 instructions against this one's 103/103.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState8SubstateSequenceScenarioScena1300_801FB1B4(void) {
  switch (D_80146875) {
  case 0:
    if (D_801460FB != 0) {
      break;
    }
    if (D_8014623B != 0) {
      break;
    }
    func_801BE1B0(0xA);
    func_8015B580(D_8014686C, 0x1D);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 4:
    func_80150224(1);
    D_80143BB0 = 2;
    D_80146875 = 6;
    break;
  case 5:
    func_80150224(0x4E);
    D_80143BB0 = 2;
    D_80146875 = 6;
    break;
  case 6:
    if (D_80143BB0 != 2) {
      func_8015C058();
      D_80146874 = 0;
      D_80146875 = 0;
    }
    break;
  case 8: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress == 0x28) {
      *progress = 0;
      D_80146875 = 0xA;
    }
    break;
  }
  case 10:
    if (D_80143BB0 != 2) {
      func_8019FA28(0x97, 0x360000, 0x1D0000, 3);
      D_80146875 = 0xB;
    }
    break;
  case 11:
    if (D_80143BB0 == 0) {
      func_8015B580(D_8014686C, 0x1E);
      func_8015C058();
      D_80146874 = 0;
      D_80146875 = 0;
      func_801A78F8();
    }
    break;
  }
}
