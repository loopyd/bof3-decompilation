#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Effect entry draw pass at 0x801F40F4, claimed by the source of that address
 * in this payload. It stamps the shared primitive cursor and draws one
 * 0x2C-byte effect entry handed in $a0. */
void drawTranslucentGouraudTriangleFromEffectEntryPositionAndTwoRotatedOffsetsTintedShadeHalfBlueWorld04Area18813_801F40F4(
    u8* entry);

/* Shared projection setup at 0x801AFE18: fills the 0x20-byte projection
 * scratch handed in $a0. Bound by the generated undefined_funcs_auto.txt row. */
void func_801AFE18(void* arg0);

/* @source 0x801F44B4
 * @behavior Overlay effect draw pass, run with no arguments and no result by
 * the two progress handlers 0x801F3598 and 0x801F364C, which call it as the
 * first of this overlay's two per-frame passes. It arms the shared draw mode
 * for the texture page GetTPage(0, 1, 0x380, 0x100) on the primitive cursor
 * g_PrimCursor with dithering off and dithering to the frame buffer on, appends
 * the 0xC-byte draw-mode primitive through 0x8014E5A0 for ordering-table index
 * 2, fills the 0x20-byte projection scratch on its own frame through 0x801AFE18,
 * and then walks the 0x10 entries of the 0x2C-byte-stride effect table at
 * 0x800E48C0: it draws the entry through 0x801F40F4 and adds 0x10 to the
 * unsigned halfword at entry byte 0x24, the spin-angle field that the same
 * shared 0x2C-byte-stride table's draw pass
 * drawTranslucentGouraudTriangleFromWorkEntryWorld04Area17213_801F3BFC feeds to
 * the sine and cosine pair 0x801782FC and 0x801783C8, and that the sibling draw
 * pass drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC advances
 * by the same step. The increment
 * is read, added and written back as an unsigned halfword and the next entry is
 * reached by advancing a byte pointer by 0x2C, while the loop counter is a byte
 * compared against 0x10.
 *
 * Evidence from the original bytes at 0x801F44B4: addiu $sp,$sp,-0x48 with sw
 * $ra,0x44, $s2,0x40, $s1,0x3C and $s0,0x38, the four addiu argument loads for
 * the GetTPage call with sw $s0,0x38 in its delay slot, then addu $a1,$zero,
 * $zero, addiu $a2,$zero,0x1 and andi $a3,$v0,0xFFFF - the zero extension of
 * the unsigned short GetTPage result into the third argument of SetDrawMode -
 * followed by the two constant pointer materialisations lui and ori 0x800E48C0
 * and 0x800E48E4 and addu $s1,$zero,$zero. Next come the g_PrimCursor load,
 * jal SetDrawMode with sw $zero,0x10 on its stack argument slot, jal
 * 0x8014E5A0 with addiu $a0,$zero,0x2 and addiu $a1,$zero,0xC, and jal
 * 0x801AFE18 with addiu $a0,$sp,0x18. The loop is jal 0x801F40F4 with addu
 * $a0,$s2,$zero, addiu $s2,0x2C, lhu $v0,0x0($s0), addiu $s1,$s1,0x1, addiu
 * $v0,$v0,0x10 and sh $v0,0x0($s0), then andi $v0,$s1,0xFF, sltiu $v0,$v0,0x10
 * and bnez with addiu $s0,0x2C in its delay slot. The second pointer is
 * 0x800E48C0 plus 0x24, the folded address of the angle halfword the loop
 * touches, so one entry pointer and its structure member reach both registers.
 *
 * Shape evidence: the sibling draw pass drawEffectEntriesThenAdvanceAngle24By10
 * World04Area17213_801F3FAC at emi/world04/area172/13@0x801F3FAC repeats the
 * whole body - the same 0x48-byte frame with $ra, $s2, $s1 and $s0 in the same
 * slots, the same GetTPage arguments, the same andi $a3,$v0,0xFFFF, the same
 * lui/ori pair for the table base plus the same folded +0x24 field pointer, the
 * same SetDrawMode and append order, the same addiu $sp,0x18 scratch argument,
 * the same addu $s1,$zero,$zero and the same andi/sltiu byte-counter loop test -
 * and differs only in the append ordering-table index, the entry count and the
 * drawn callee. A byte-identical scan of every payload under out/binaries finds
 * this 180-byte body only at 0x18B4 of its own payload, so the range has no
 * clone twin and the sibling contributes shape, not bytes.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEffectEntriesThenAdvanceAngle24By10World04Area18813_801F44B4(void) {
  u8 scratch[0x20];
  u8* entry;
  u8 index;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  GpuAppendPrim(2u, 0x0cu);
  func_801AFE18(scratch);

  entry = PSX_PTR(u8, 0x800E48C0u);
  index = 0u;

  do {
    drawTranslucentGouraudTriangleFromEffectEntryPositionAndTwoRotatedOffsetsTintedShadeHalfBlueWorld04Area18813_801F40F4(
        entry);
    FIELD_REF(u16, entry, 0x24u) = (u16)(FIELD_REF(u16, entry, 0x24u) + 0x10);
    entry += 0x2cu;
    index += 1u;
  } while (index < 0x10u);
}
