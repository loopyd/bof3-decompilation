#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801F7BFC
 * @behavior Word 2 of the overlay-local second-level handler run at 0x801FAEFC,
 * which the exact dispatcher dispatchSecondaryStateScenarioScena1100_801F7748
 * (0x801F7748) reaches through the shared primary state byte D_80146874; no jal
 * or j in the payload targets 0x801F7BFC, so that table word is its only
 * reference. It takes the address of the shared secondary sub-state byte
 * D_80146875 (0x80146875) once, into a local pointer, reads it with `lbu` once
 * as the switch selector and runs one of two sequences over it. Sub-state 0,
 * while the shared front-end mode byte D_80143BB0 (0x80143BB0, read with `lbu`)
 * is not 2, stores 1 in the shared scenario progress byte g_ScenarioProgress
 * (0x80146864), runs the shared GAME.EMI helper func_801BE1B0 (0x801BE1B0)
 * with argument 0 and advances the sub-state byte to 1. Sub-state 1, once the
 * progress byte reads 3, sets flag bit 5 of the shared flag word D_8014686C
 * (0x8014686C, read with `lw`) through the shared flag helper func_8015B580
 * (0x8015B580) with id 5, runs the shared overlay release helper func_8015C058
 * (0x8015C058), clears the shared bytes 0x80146865, 0x80146866 and 0x80146867,
 * clears the sub-state byte D_80146875 and finally clears the shared primary
 * state byte D_80146874 (0x80146874), ending the chain. Every other sub-state
 * value returns without touching any byte. It takes no arguments and returns
 * nothing; its 0x20-byte frame keeps $ra, $s0 and $s1 because the pointer to the
 * sub-state byte stays live in $s1 across the whole body and the constant 1 is
 * hoisted into $s0 before the func_801BE1B0 call so the one materialisation
 * feeds both the progress store and the later sub-state store. The constant 1
 * for the sub-state selector compare is materialised in the delay slot of the
 * sub-state 0 test and the constant 3 for the progress compare in the delay
 * slot of the sub-state 1 test, so the two stores of 1 and the compares of 1 and
 * 3 place the switch comparisons in the order source `switch` clauses give. The
 * five byte stores of the sub-state 1 tail keep the order 0x80146865,
 * 0x80146866, 0x80146867, D_80146875, D_80146874, each of the four absolute
 * ones materialising the 0x8014 page in $at on its own while the sub-state store
 * reuses the $s1 base. Its 0xCC bytes have no copy in the tree: a scan of every
 * payload under the extracted binaries finds the span only at its own offset,
 * and the closest unrelated 51-word window (scena05 0x801F848C) shares 18 of its
 * words. The nearest shape sibling is the exact scena09 state machine at
 * 0x801FA08C, which supplies the address-taken sub-state pointer, the `switch`
 * chain and the identical clearing tail of the shared release helper followed
 * by sub-state 0 and D_80146874 = 0, while this body differs in every case set,
 * test and constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armProgress1UnlessMode2ThenWaitProgress3SetFlagBit5AndClearStateScenarioScena1100_801F7BFC(void) {
  u8 *state = &D_80146875;

  switch (*state) {
  case 0:
    if (D_80143BB0 == 2u) {
      return;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(0u);
    *state = 1u;
    return;
  case 1:
    if (g_ScenarioProgress != 3u) {
      return;
    }
    func_8015B580(D_8014686C, 5);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    *state = 0u;
    D_80146874 = 0u;
    return;
  default:
    return;
  }
}
