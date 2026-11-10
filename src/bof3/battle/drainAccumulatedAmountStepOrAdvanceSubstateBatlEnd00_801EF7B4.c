#include "bof3/bof3.h"

extern u16 D_80145AA4;
extern u8 D_801462E4;
extern s32 D_80146330;
extern u16 D_801463B0;

/* EXE-side EMI loader ready test: the same-target dispatchers at 0x801EEF14,
 * 0x801EEFCC and 0x801EF870 poll it with no arguments and branch on a nonzero
 * answer. */
s32 func_80162D00(void); /* @source 0x80162D00 */

/* Shared two-argument helper of the main executable: the same payload uses it
 * to take an amount and a zero flag (the scenario08 lift of 0x801FB8A0 runs it
 * as func_8016666C(0xBB8, 0)), and this body passes its own amounts with the
 * same zero flag. Its own role is not established, so it stays declared for its
 * address and the answer it returns is discarded here. */
void func_8016666C(s32 arg0, s32 arg1); /* @source 0x8016666C */

/* @source 0x801EF7B4
 * @behavior Battle-end accumulated-amount drain: the pad state word at
 *   0x80145AA4 reading nonzero together with the EXE-side EMI loader reporting
 *   ready at 0x80162D00 finishes the drain at once, so the whole remaining
 *   amount in the accumulator at 0x80146330 is passed to the shared helper at
 *   0x8016666C with a zero second argument, that accumulator is cleared and the
 *   substate index byte at 0x801462E4 advances by one. Otherwise, while the
 *   amount still exceeds the per-call step published at 0x801463B0, only that
 *   step is passed to 0x8016666C and subtracted from the accumulator, and the
 *   substate index stays put; once the remainder fits inside one step it is
 *   drained whole the same way the skip path does. The accumulator word and the
 *   step halfword are both re-read for the subtraction that follows the call, so
 *   the amount is never held across it and the difference is computed twice.
 *   Takes no arguments and returns nothing; the 0x18-byte frame keeps $ra and
 *   $s0 across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801EF7B4 is entry 13 (index 12) of the fourteen-pointer run based at
 * 0x801F0C98 (textbin D_801F0C98); the payload's own word at 0x801F0CC8 names
 * it, and dispatchSubstateTable0c98BatlEnd00_801EEC58 indexes that run with the
 * byte at 0x801462E3. The substate index byte this body advances, 0x801462E4,
 * is the one the run-local dispatchers scale into the handler runs based at
 * 0x801F0CA4/0x801F0CB8, so the drain is the last step of that run before
 * gateCountedListEntriesThenEnterPhase4BatlEnd00_801EF870 changes phase.
 * The accumulator at 0x80146330 is the pool accumulateEnemyHalfwordsAndResetScratch
 * adds the enemy work halfword +0x84 into, next to the pool at 0x8014632C that
 * the sibling drain at 0x801EEF14 empties in the same steps into the template
 * accumulator at 0x801DD59C; 0x801463B0 is the step halfword that body also
 * reads, and the shared-address lifts of the sibling battle overlays
 * (batl_ovr/00) seed it to 0x2C through
 * initBattleScenePrimsAndAdvanceSubstate and raise it by four up to 0xF4 as
 * the x of the battle-end panel that func_801EEF10 draws.
 */
void drainAccumulatedAmountStepOrAdvanceSubstateBatlEnd00_801EF7B4(void) {
  s32* amount;
  u8 next;

  if ((D_80145AA4 != 0) && (func_80162D00() != 0)) {
    amount = &D_80146330;
    func_8016666C(*amount, 0);
    next = D_801462E4;
    *amount = 0;
    D_801462E4 = next + 1;
    return;
  }
  amount = &D_80146330;
  if (*amount - D_801463B0 > 0) {
    func_8016666C(D_801463B0, 0);
    *amount = *amount - D_801463B0;
    return;
  }
  func_8016666C(*amount, 0);
  next = D_801462E4;
  *amount = 0;
  D_801462E4 = next + 1;
}
