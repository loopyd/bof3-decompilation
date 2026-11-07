#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_801D147C(u8* origin, s16 x, s32 flags);

/* @source 0x801D0CC0
 * @behavior Entry 1 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It adds 0x20 to the unsigned halfword at +0x2E of that object,
 * stores the truncated halfword back and hands the overlay helper at
 * 0x801D147C the object's origin block at +0x64, that updated value read as a
 * signed halfword, and the flag mask 6. It then ticks the object's countdown
 * byte at +0x9 down by one; when that byte reaches zero it re-arms it with 0x40
 * and advances the work byte at +0x1 by one, so the next dispatch selects entry
 * 2 (0x801D0D54). All three later scratchpad accesses are separate loads of the
 * 0x1F800044 cell because the halfword and byte stores through the loaded
 * pointer may alias the cell. It takes no arguments, returns nothing and its
 * 0x18-byte frame exists only to keep $ra at 0x10 across the helper call.
 *
 * Evidence: the word at 0x801D391C names 0x801D0CC0 and the following words
 * 0x801D3920, 0x801D3924, 0x801D3928 and 0x801D392C name 0x801D0D54,
 * 0x801D0DD8, 0x801D0F10 and 0x801D0FEC, so this body is element 1 of the run
 * whose base 0x801D3918 is loaded by `lui $at,0x801D` / `addu $at,$at,$v0` /
 * `lw $v0,0x3918($at)` in the overlay entry after `sll $v0,$v0,2` of the work
 * byte at +0x1. The helper's arguments are read off 0x801D147C: it keeps $a0
 * in $s1 and derives its primitive writes from it, stores $a1 into two stack
 * halfwords with `sh`, and splits $a2 with `andi ...,0x4`, `andi ...,0x2` and
 * `andi ...,0x1`, so the third argument is a flag mask, not a count.
 *
 * Shape evidence: the next span func_801D0D54 is the same statement sequence
 * without the halfword advance and with the countdown re-armed with 0x20, so
 * its statement order, operand forms and declaration style carry over here. The
 * `lhu` / `addiu` / `sh` / `sll` / `sra` advance through halfword +0x2E is the
 * one func_801D0DD8 makes with -0x6, so the object keeps an unsigned halfword
 * field there and the signed halfword handed to the helper is the conversion of
 * that new value. The exact lift
 * countdownReload32ThenAdvanceHandlerScenarioScena0700_801F8730 (0x801F8730 in
 * emi/scenario/scena07/00) has the identical tail, including the
 * `andi $v0,$v0,0xFF` that the promoted `u8` decrement produces, and the exact
 * lift decrementWorkByte9Reload252AndAdvanceStateScenarioScena0100_801F752C
 * (0x801F752C) records the same re-read of the pointer cell after every store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHalfword2EThenInvokeOriginHelperTickCountdownReload64AndAdvanceWorkByte1ScenarioSce15ef100_801D0CC0(void) {
  u8* work;
  u16 advance;

  work = D_1F800044;
  advance = *(u16*)(work + 0x2E) + 0x20;
  *(u16*)(work + 0x2E) = advance;
  func_801D147C(work + 0x64, advance, 6);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x40;
    D_1F800044[1]++;
  }
}
