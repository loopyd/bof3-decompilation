#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_801D147C(u8* origin, s16 x, s32 flags);

/* @source 0x801D0D54
 * @behavior Entry 2 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It hands the overlay helper at 0x801D147C the work object's origin
 * block at +0x64, the signed halfword at +0x2E and the flag mask 6, then ticks
 * the object's countdown byte at +0x9 down by one; when that byte reaches zero
 * it re-arms it with 0x20 and advances the work byte at +0x1 by one, so the
 * next dispatch selects entry 3 (0x801D0DD8). All three scratchpad accesses are
 * separate loads of the 0x1F800044 cell because the byte store through the
 * loaded pointer may alias the cell. It takes no arguments, returns nothing and
 * its 0x18-byte frame exists only to keep $ra at 0x10 across the helper call.
 *
 * Evidence: the four words at 0x801D3920, 0x801D3924, 0x801D3928 and 0x801D392C
 * name 0x801D0D54, 0x801D0DD8, 0x801D0F10 and 0x801D0FEC, so this body is
 * element 2 of the run whose base 0x801D3918 is loaded by `lui $at,0x801D` /
 * `addu $at,$at,$v0` / `lw $v0,0x3918($at)` in the overlay entry after
 * `sll $v0,$v0,2` of the work byte at +0x1. The helper's arguments are read off
 * 0x801D147C: it keeps $a0 in $s1 and derives its primitive writes from it,
 * stores $a1 into two stack halfwords with `sh`, and splits $a2 with
 * `andi ...,0x4`, `andi ...,0x2` and `andi ...,0x1`, so the third argument is a
 * flag mask, not a count.
 *
 * Shape evidence: the neighbouring span func_801D0CC0 is the same statement
 * sequence with the halfword at +0x2E advanced by 0x20 before the call and the
 * countdown re-armed with 0x40, so the order of the helper call, the
 * decrement-and-test of byte +0x9 and the reload/re-arm pair is that sibling's.
 * The exact lift countdownReload32ThenAdvanceHandlerScenarioScena0700_801F8730
 * (0x801F8730 in emi/scenario/scena07/00) has the identical tail, including the
 * `andi $v0,$v0,0xFF` that the promoted `u8` decrement produces, and the exact
 * lift decrementWorkByte9Reload252AndAdvanceStateScenarioScena0100_801F752C
 * (0x801F752C) records the same re-read of the pointer cell after every store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeOriginHelperThenTickCountdownReload32AndAdvanceWorkByte1ScenarioSce15ef100_801D0D54(void) {
  u8* work;

  work = D_1F800044;
  func_801D147C(work + 0x64, *(s16*)(work + 0x2E), 6);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x20;
    D_1F800044[1]++;
  }
}
