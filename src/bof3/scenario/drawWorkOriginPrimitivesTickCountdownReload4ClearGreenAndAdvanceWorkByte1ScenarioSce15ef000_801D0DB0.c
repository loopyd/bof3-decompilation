#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_801D1A20(u8* origin, s16 angle, s32 flags);
void func_801D1C1C(u8* origin, u8* tint, u8 red);

/* @source 0x801D0DB0
 * @behavior Entry 2 of the overlay's step-handler run at 0x801D1F88, which the
 * overlay entry func_801D0C04 indexes with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044
 * (`sll $v0,$v0,2` then `lw` from the run base and `jalr`). It takes no
 * arguments and returns nothing. It hands this overlay's first drawing helper
 * func_801D1C1C (0x801D1C1C) the work object's origin block at +0x34, the
 * block at +0xC and the tint byte at +0x5D, then hands the second drawing
 * helper func_801D1A20 (0x801D1A20) the same origin block at +0x34, the
 * signed halfword at +0x2E and the mode constant 2. It finally ticks the work
 * object's countdown byte at +0x9 down by one and, when that byte reaches
 * zero, clears the colour channel byte at +0x5E, re-arms the countdown to 4
 * and advances the work byte at +0x1 by one, so the next frame dispatches the
 * following run entry. Its 0x18-byte frame keeps $ra at 0x10 and nothing else
 * live across the two calls.
 *
 * Evidence: the run word at 0x801D1F90 is 0x801D0DB0 and the surrounding words
 * hold 0x801D0C48, 0x801D0CEC, 0x801D0E5C, 0x801D0F44, 0x801D101C, 0x801D10C4
 * and 0x801D11B4, so this body is element 2 of the run whose base 0x801D1F88
 * func_801D0C04 loads. This address has no in-payload `jal` and no other word
 * in the payload holds it, so the entry is reached only through that run. Both
 * callees open with GetTPage and SetDrawMode and consume g_PrimCursor, and
 * func_801D1C1C keeps its third argument in $s3 for the primitive it submits,
 * which fixes the drawing role and the argument widths: the third argument of
 * the first call is the `lbu` at +0x5D, and the second argument of the second
 * call is the `lh` at +0x2E, that is a signed halfword. The
 * `lbu`/`addiu -1`/`sb`/`andi 0xFF`/`bnez` countdown and the three byte
 * stores of the guard block are the tail of the exact sibling
 * advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8
 * in target emi/scenario/sce15ef1/00, with this body's own re-arm 4, channel
 * +0x5E and no table walk. The three colour channels are the ones the exact
 * lift drawWorkObjectTintSprite maps as 0x5D/0x5E/0x5F into an SPRT's r0/g0/b0,
 * so +0x5E is the green channel and +0x5D the red one.
 *
 * Shape decisions measured on this body: addressing the cell inline in each
 * statement is the shape the original shows, because within one statement the
 * repeated cell reads fold into the single `lw` the head and the second call
 * each issue. One `u8* work` local reused for both calls measured 34/43 with
 * the first difference at the entry `lui`: a single local is one allocno and
 * put both loads in `$v0`, where the original uses `$a1` then `$a0`. Two
 * distinct locals and the inline form each measured 43/43; the inline form is
 * kept. The countdown block addresses the cell directly as the exact sibling
 * does, and the constant 2 precedes the `lh` the way the original's
 * `addiu $a2,$zero,2` does.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkOriginPrimitivesTickCountdownReload4ClearGreenAndAdvanceWorkByte1ScenarioSce15ef000_801D0DB0(void) {
  func_801D1C1C(D_1F800044 + 0x34, D_1F800044 + 0xC, D_1F800044[0x5D]);
  func_801D1A20(D_1F800044 + 0x34, *(s16*)(D_1F800044 + 0x2E), 2);
  if (--D_1F800044[9] == 0) {
    D_1F800044[0x5E] = 0;
    D_1F800044[9] = 4;
    D_1F800044[1]++;
  }
}
