#include "bof3/bof3.h"

extern u16 D_80145AA4;
extern u8 D_801462E4;
extern s32 D_8014632C;
extern u16 D_801463B0;

/* EXE-side EMI loader ready test: the same-target dispatchers at 0x801EEFCC and
 * 0x801EF870 poll it with no arguments and branch on a nonzero answer. */
s32 func_80162D00(void); /* @source 0x80162D00 */

/* Shared accumulator helper of the main executable: it adds its argument into
 * the third word of every eligible local work record's 164-byte template
 * record and returns that argument (lifted exact at this address as
 * accumulateEligibleTemplateWord in the battle/03 target). This body only
 * wants the accumulation, so it discards the answer. */
s32 func_801DD59C(s32 arg0); /* @source 0x801DD59C */

/* @source 0x801EEF14
 * @behavior Battle-end accumulated-amount drain: the pad state word at
 *   0x80145AA4 reading nonzero together with the EXE-side EMI loader reporting
 *   ready at 0x80162D00 finishes the drain at once, so the whole remaining
 *   amount in the accumulator at 0x8014632C is passed to the shared template
 *   accumulator at 0x801DD59C, that accumulator is cleared and the substate
 *   index byte at 0x801462E4 advances by one. Otherwise, while the amount still
 *   exceeds the per-call step published at 0x801463B0, only that step is passed
 *   to 0x801DD59C and subtracted from the accumulator, and the substate index
 *   stays put; once the remainder fits inside one step it is drained whole the
 *   same way the skip path does. The accumulator word and the step halfword are
 *   both re-read for the subtraction that follows the call, so the amount is
 *   never held across it and the difference is computed twice. Takes no
 *   arguments and returns nothing; the 0x18-byte frame keeps $ra and $s0 across
 *   the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801EEF14 is entry 6 of the pointer run based at 0x801F0C98 that
 * dispatchSubstateTable0c98BatlEnd00_801EEC58 indexes with the byte at
 * 0x801462E3, and the substate byte it advances is the one the run-local
 * dispatchers scale into the handler runs based at 0x801F0CA4/0x801F0CB8.
 * The amount at 0x8014632C is the pool accumulateEnemyHalfwordsAndResetScratch
 * accumulates the enemy work halfwords +0x86 and +0x84 into and
 * func_801EECD0 divides by countEligibleBattlersBatlEnd00_801EEDA8; the step at
 * 0x801463B0 is what the entry before this one, func_801EEE4C, publishes as
 * that amount divided by 0x1E with a floor of one.
 */
void applyAccumulatedAmountStepOrAdvanceSubstateBatlEnd00_801EEF14(void) {
  s32* amount;
  u8 next;

  if ((D_80145AA4 != 0) && (func_80162D00() != 0)) {
    amount = &D_8014632C;
    func_801DD59C(*amount);
    next = D_801462E4;
    *amount = 0;
    D_801462E4 = next + 1;
    return;
  }
  amount = &D_8014632C;
  if (*amount - D_801463B0 > 0) {
    func_801DD59C(D_801463B0);
    *amount = *amount - D_801463B0;
    return;
  }
  func_801DD59C(*amount);
  next = D_801462E4;
  *amount = 0;
  D_801462E4 = next + 1;
}
