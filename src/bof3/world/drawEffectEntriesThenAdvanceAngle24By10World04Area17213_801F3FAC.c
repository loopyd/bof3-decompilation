#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Effect entry draw pass at 0x801F3BFC: stamps the shared primitive cursor,
 * turns the semi-transparency on, appends the entry's rotated four-corner
 * primitive through 0x801AFF04 and builds its rotation from the entry spin angle
 * halfword at 0x24, the extents at 0x10 and 0x12 and the scale halfword at 0x28,
 * so it is the caller of the sine and cosine pair 0x801782FC and 0x801783C8.
 * Still an asm boundary in this payload, so it keeps its address anchor. */
void func_801F3BFC(u8* entry);

/* Shared projection setup at 0x801AFE18: fills the 0x20-byte projection scratch
 * handed in $a0. Bound by the generated undefined_funcs_auto.txt row. */
void func_801AFE18(void* arg0);

/* @source 0x801F3FAC
 * @behavior Overlay effect draw pass, run by the state byte 1 handler entry 4
 *           at 0x801F32E8 with no arguments and no result. It arms the shared
 *           draw mode for the texture page GetTPage(0, 1, 0x380, 0x100) on the
 *           primitive cursor g_PrimCursor with dithering off and dithering to
 *           the frame buffer on, appends the 0xC-byte draw-mode primitive
 *           through 0x8014E5A0 for ordering-table index 1, fills the 0x20-byte
 *           projection scratch on its own frame through 0x801AFE18, and then
 *           walks all 0x20 entries of the 0x2C-byte-stride effect table at
 *           0x800E48C0: it draws the entry through 0x801F3BFC and, with the
 *           entry base still live, adds 0x10 to the unsigned halfword at entry
 *           byte 0x24, which is the spin angle the draw pass feeds to the sine
 *           and cosine pair at 0x1000 per turn. The increment is read, added and
 *           written back as an unsigned halfword and the next entry is reached
 *           by advancing a byte pointer by 0x2C, while the loop counter is a
 *           byte compared against 0x20.
 *
 * Evidence from the original bytes at 0x801F3FAC: addiu $sp,$sp,-0x48 with sw
 * $ra,0x44, $s2,0x40, $s1,0x3C and $s0,0x38, the four addiu argument loads for
 * the GetTPage call with sw $s0,0x38 in its delay slot, then addu $a1,$zero,
 * $zero, addiu $a2,$zero,0x1 and andi $a3,$v0,0xFFFF - the zero extension of
 * the unsigned short GetTPage result into the signed third argument of
 * SetDrawMode - followed by the two constant pointer materialisations lui and
 * ori 0x800E48C0 and 0x800E48E4 and addu $s1,$zero,$zero. Next come the
 * g_PrimCursor load, jal SetDrawMode with sw $zero,0x10 on its stack argument
 * slot, jal 0x8014E5A0 with addiu $a1,$zero,0xC, and jal 0x801AFE18 with addiu
 * $a0,$sp,0x18. The loop is jal 0x801F3BFC with addu $a0,$s2,$zero, addiu
 * $s2,0x2C, lhu $v0,0x0($s0), addiu $s1,$s1,0x1, addiu $v0,$v0,0x10 and
 * sh $v0,0x0($s0), then andi $v0,$s1,0xFF, sltiu $v0,$v0,0x20 and bnez with
 * addiu $s0,0x2C in its delay slot. The second pointer is 0x800E48C0 plus 0x24,
 * the folded address of the angle halfword the loop touches, so one entry
 * pointer and its structure member reach both registers.
 *
 * Shape evidence: the exact sibling drawSpin at
 * emi/world00/area024/14@0x801F3D5C in this tree repeats the whole prologue -
 * the same 0x48-byte frame with $ra, $s2, $s1 and $s0 in the same slots, the
 * same GetTPage arguments, the same andi $a3,$v0,0xFFFF, the same
 * lui/ori pair for the table base plus a second folded field pointer, the same
 * SetDrawMode and append order, the same addiu $sp,0x18 scratch argument and
 * the same addu $s1,$zero,$zero with andi/sltiu loop test over a byte counter -
 * and differs only in its dithering argument, its table base, its entry count,
 * its drawn field offsets and the extra scratch-byte branch it carries. The
 * byte-identical sibling search over every payload under out/binaries finds no
 * twin for this range, unmasked or with the call immediate words masked.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC(void) {
  u8 scratch[0x20];
  u8* entry;
  u8 index;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  GpuAppendPrim(1u, 0x0cu);
  func_801AFE18(scratch);

  entry = PSX_PTR(u8, 0x800E48C0u);
  index = 0u;

  do {
    func_801F3BFC(entry);
    FIELD_REF(u16, entry, 0x24u) = (u16)(FIELD_REF(u16, entry, 0x24u) + 0x10);
    entry += 0x2cu;
    index += 1u;
  } while (index < 0x20u);
}
