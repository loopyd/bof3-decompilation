#include "bof3/bof3.h"

/* Shared main-RAM flag-bank word: this body reads it with `lw` and hands the
 * word itself to the EXE-side flag setter below. It sits below this overlay's
 * load address and resolves through this target's generated undefined-symbol
 * map, which already names it. */
extern u32 D_8014686C; /* @source 0x8014686C @kind data */

/* Shared main-EXE helpers, both outside this payload and unlifted, so only the
 * call-site prototypes are declared. 0x80166180 is the setter half of the
 * selector query/set pair whose read half is 0x80166140; 0x8015B580 sets one
 * flag of the byte bank it is handed. */
void func_80166180(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801F33B0
 * @behavior Slot 11 of this overlay's handler table at 0x801F52B0, whose 12th
 *           word 0x801F52DC (payload offset 0x26DC) is this function's only
 *           reference: no `jal` and no branch in the payload targets 0x801F33B0,
 *           so its entry roles stay structural and it takes no arguments and
 *           returns nothing. It first marks shared selector id 7 through the
 *           main-EXE setter func_80166180, then sets flag 0x1E (30) of the
 *           shared flag-bank word D_8014686C, read with `lw`, through the
 *           EXE-side flag setter func_8015B580. Nothing else is read or
 *           written. The 0x18-byte frame keeps only $ra at 0x10 across the two
 *           calls, and the two constant arguments are the `addiu $a0,$zero,0x7`
 *           and `addiu $a1,$zero,0x1E` delay-slot immediates of those calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: 0x7 is the argument of the selector setter and 0x1E the flag
 * index handed to the flag setter, so the name records the two writes and
 * invents no game-state role for either. The helper roles and the wording
 * "setter of one flag of a byte bank at the address it is handed" are the ones
 * in-tree siblings carried when their own gates measured exact:
 * setSharedFlag23WhenSharedSelector5IsSetWorld02Area09713_801F2D28 and
 * armProgress1UnlessMode2ThenWaitProgress3SetFlagBit5AndClearStateScenarioScena1100_801F7BFC.
 *
 * Shape evidence: statement order, operand forms and declaration style follow
 * those two exact siblings, whose 0x18-byte frames also keep only $ra, and the
 * flag word is bound as the u32 cell that the armProgress1 body read with
 * `lw` before passing it to func_8015B580. No byte-identical sibling exists:
 * the 12-word pattern of this span occurs once in the 398 payloads under
 * out/binaries, at offset 0x7B0 of this one, so the clone lever is declined
 * and the body is seeded from the original bytes.
 */
void markFlagFlagWord(
    void) {
  func_80166180(7);
  func_8015B580(D_8014686C, 0x1E);
}
