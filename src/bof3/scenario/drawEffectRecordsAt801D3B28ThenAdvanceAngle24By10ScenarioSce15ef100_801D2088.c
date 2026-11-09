#include "bof3/bof3.h"

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Table of 0x20 records of 0x2C bytes at 0x801D3B28, directly after the
 * 0x10-entry 0x18-byte-stride record table at 0x801D39A8 and inside the same
 * Splat `textbin` row. It is zero on disc and filled at runtime: seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68
 * seeds one record from the scratchpad work object's position triple at +0x34,
 * its random angle fields and the halfword pair at +0x28 and +0x2A, and clears
 * the spin angle at +0x24. */
extern u8 D_801D3B28[]; /* @source 0x801D3B28 @kind data */

/* Primitive submit helper of the shared main executable: it takes the ordering
 * table slot and the primitive size as its two byte arguments. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* Projection setup helper of the shared main executable: it fills the 0x20-byte
 * scratch block handed in $a0. */
void func_801AFE18(void* arg0);

/* Record draw helper of this payload at 0x801D1CD4: it stamps the primitive
 * cursor, turns semi-transparency on, projects the record position triple at
 * +0x0/+0x4/+0x8 and builds the rotated quad from the spin angle halfword at
 * +0x24 through the sine and cosine pair 0x801783C8 and 0x801782FC, the extents
 * at +0x10/+0x12, the scale halfword at +0x28 and the second scale halfword at
 * +0x14. Still an asm boundary in this payload, so it keeps its address anchor. */
void func_801D1CD4(u8* record);

/* @source 0x801D2088
 * @behavior Draw pass over this overlay's 0x2C-byte effect records at
 *           0x801D3B28, which the four step handlers func_801D0F10,
 *           func_801D0FEC, func_801D106C and func_801D12A0 call right after the
 *           record pass 0x801D1A60 and which takes no arguments and returns no
 *           result. It first arms the shared draw mode for the texture page
 *           GetTPage(0, 1, 0x380, 0x100) on the primitive cursor g_PrimCursor
 *           with dithering off and dithering to the frame buffer on, appends the
 *           0xC-byte draw-mode primitive through 0x8014E5A0 for ordering-table
 *           slot 2 so that it precedes the semi-transparent records of that
 *           slot, and fills the 0x20-byte projection scratch of its own frame
 *           through 0x801AFE18. It then walks all 0x20 records of the table at
 *           0x801D3B28: it draws the record through 0x801D1CD4 and, with the
 *           record base still live, adds 0x10 to the unsigned halfword at record
 *           byte 0x24, the spin angle the draw helper feeds to the sine and
 *           cosine pair. Both the increment and the next-record step happen
 *           after the draw of the current record; the next record is reached by
 *           advancing a byte pointer by 0x2C and the loop counter is a byte
 *           compared against 0x20. The 0x48-byte frame holds the 0x10-byte
 *           outgoing argument area, the 0x20-byte projection scratch at +0x18,
 *           the saved $ra at +0x44 and $s0-$s2 at +0x38, +0x3C and +0x40.
 *
 * Evidence from the original bytes at 0x801D2088: addiu $sp,$sp,-0x48 with the
 * four addiu argument loads for GetTPage (0, 1, 0x380, 0x100) and sw $s0,0x38 in
 * that call's delay slot, then addu $a1,$zero,$zero, addiu $a2,$zero,0x1 and
 * andi $a3,$v0,0xFFFF - the zero extension of the unsigned short GetTPage result
 * into SetDrawMode's integer texture-page argument - followed by lui/addiu of
 * 0x801D3B28 into $s1, addiu $s0,$s1,0x24 for the folded angle-field pointer,
 * addu $s2,$zero,$zero, the g_PrimCursor load and jal SetDrawMode with
 * sw $zero,0x10 on its stack argument slot. Next come jal 0x8014E5A0 with
 * addiu $a0,$zero,0x2 and addiu $a1,$zero,0xC, and jal 0x801AFE18 with
 * addiu $a0,$sp,0x18. The loop is jal 0x801D1CD4 with addu $a0,$s1,$zero,
 * addiu $s1,$s1,0x2C, lhu $v0,0x0($s0), addiu $s2,$s2,0x1, addiu $v0,$v0,0x10
 * and sh $v0,0x0($s0), then andi $v0,$s2,0xFF, sltiu $v0,$v0,0x20 and bnez with
 * addiu $s0,$s0,0x2C in its delay slot, and the epilogue restores the four saved
 * registers from the same slots and returns.
 *
 * Name evidence: the drawn table is the 0x2C-byte-stride record run at
 * 0x801D3B28 the exact lift of 0x801D0DD8 already walks and seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68 seeds,
 * and byte 0x24 of a record is its spin angle: the seed clears it, the draw
 * helper 0x801D1CD4 reads it as the argument of the sine and cosine pair
 * 0x801783C8 and 0x801782FC, and this body adds 0x10 to it once per record per
 * pass. The only in-image references to 0x801D2088 are the jal sites in the four
 * step handlers above, so no table word and no script blob publishes the address.
 *
 * Shape evidence: the exact lift
 * drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC repeats the
 * whole body - the same 0x48-byte frame with $ra, $s2, $s1 and $s0 in the same
 * slots, the same GetTPage arguments, the same andi $a3,$v0,0xFFFF, the same
 * table base plus folded angle-field pointer pair, the same SetDrawMode and
 * append order and the same addiu $sp,0x18 scratch argument - and differs only
 * in its table base, its draw call, its ordering-table slot and its table size.
 * That sibling is shape evidence, not a clone: its table base materialises as
 * lui plus ori of the integer address 0x800E48C0 while this body materialises
 * lui plus addiu of the symbol 0x801D3B28, and its calls name other addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088(
    void) {
  u8 scratch[0x20];
  u8* record;
  u8 index;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(2u, 0x0cu);
  func_801AFE18(scratch);

  record = D_801D3B28;
  index = 0u;

  do {
    func_801D1CD4(record);
    FIELD_REF(u16, record, 0x24u) = (u16)(FIELD_REF(u16, record, 0x24u) + 0x10);
    record += 0x2cu;
    index += 1u;
  } while (index < 0x20u);
}
