#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original materialises
 * it once with a `lui 0x1F80` plus a load at displacement 0x44 ahead of the
 * frame setup and reuses that one base for the three position-word loads, so
 * the cell binds by its own fixed address with no map row. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

extern int rand(void);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products the drawing passes of this payload
 * form, and this body takes the 0x801783C8 sample for x and the 0x801782FC
 * sample for y. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared GTE matrix helpers of the main executable: 0x801AFF64 seeds the
 * scratch rotation matrix and the three rotation helpers post-multiply one axis
 * each, the middle one through the negated angle. */
void func_801AFF64(void* arg0);
void func_80179C58(s32 arg0, MATRIX* arg1);
void func_80179DF8(s32 arg0, MATRIX* arg1);
void func_80179F98(s32 arg0, MATRIX* arg1);

/* @source 0x801F4394
 * @behavior Effect entry seeder, called once for each of the 0x10 entries of
 *   the 0x2C-byte-stride effect table at 0x800E48C0 by the progress handler
 *   0x801F3484 on the frame its countdown expires. It copies the 0x34, 0x38 and
 *   0x3C position words of the scratchpad work object behind 0x1F800044 into
 *   the entry words at +0x00, +0x04 and +0x08, seeds the corner offset SVECTORs
 *   at +0x10 and +0x18 from the shared angular pair 0x801783C8/0x801782FC at
 *   +0x10 and -0x10 with their third halfwords cleared, takes three random
 *   Euler angles (rand() & 0xFFF, (rand() & 0x7FF) - 0x400 and rand() & 0xFFF),
 *   builds the scratch rotation matrix through 0x801AFF64 and the three shared
 *   rotation helpers 0x80179C58, 0x80179DF8 with the middle angle negated and
 *   0x80179F98, and rotates both corner offset SVECTORs through that matrix in
 *   place with ApplyMatrixSV. It then arms the entry scale halfword at +0x28 to
 *   (rand() & 0x2) + 0x3, clears the spin angle at +0x24 together with the
 *   halfwords at +0x20 and +0x22, and writes the constant 0x20 into the shade
 *   halfword at +0x2A. The draw pass func_801F40F4 of this payload reads the
 *   position words as the quad base, the two offset SVECTORs as the quad
 *   extents and the +0x24 angle as the spin, scales the extents by the +0x28
 *   halfword, and clamps +0x2A into the 0..0xFF range of the red and green
 *   bytes it stamps with blue at half that value.
 *
 * Evidence from the original bytes at 0x801F4394: `lui $v1,0x1F80` plus
 * lw 0x44($v1) before the 0x40-byte frame, addiu $sp,$sp,-0x40 with sw $ra,0x3C
 * and sw $s0,0x38, then lw $v0,0x34($v1) with addu $s0,$a0,$zero, sw $v0,0x0($s0),
 * lw $v0,0x38($v1), nop and sw $v0,0x4($s0), and lw $v0,0x3C($v1) with
 * jal rand carrying sw $v0,0x8($s0) in its delay slot. The three angle reads are
 * andi $v0,$v0,0xFFF with sh 0x30($sp), andi $v0,$v0,0x7FF with
 * addiu $v0,$v0,-0x400 and sh 0x32($sp), and andi $v0,$v0,0xFFF with
 * sh 0x34($sp). The corner seeds are jal 0x801783C8 with addiu $a0,$zero,0x10
 * and sh 0x10($s0), jal 0x801782FC with addiu $a0,$zero,0x10 and sh 0x12($s0),
 * jal 0x801783C8 with addiu $a0,$zero,-0x10 and sh $zero,0x14($s0),
 * jal 0x801782FC with addiu $a0,$zero,-0x10 and sh 0x18($s0), then
 * sh 0x1A($s0) and sh $zero,0x1C($s0) around jal 0x801AFF64 with
 * addiu $a0,$sp,0x10 in its delay slot. The rotations reload each angle from the
 * stack - lh $a0,0x30($sp), lh $a0,0x32($sp) with negu $a0,$a0 in the
 * jal 0x80179DF8 delay slot, lh $a0,0x34($sp) - and pass addiu $a1,$sp,0x10.
 * The two ApplyMatrixSV calls take addiu $a0,$sp,0x10 with addiu $a1,$s0,0x10
 * and addiu $a1,$s0,0x18, each followed by addu $a2,$a1,$zero, so both calls
 * pass the same in-place vector twice. The tail is jal rand with nop in its
 * delay slot, andi $v0,$v0,0x2 and addiu $v0,$v0,0x3 into sh 0x28($s0),
 * addiu $v0,$zero,0x20, then sh $zero to 0x20($s0), 0x22($s0) and 0x24($s0)
 * and sh $v0,0x2A($s0).
 *
 * Seed evidence: a payload comparison finds the exact scenario lift
 * seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68
 * at 0x801D1F68 of emi/scenario/sce15ef1/00 holding this whole 0x120-byte body
 * with one differing word out of the 0x48 - the addiu that loads the +0x2A
 * constant, 0x20 here and 0x40 there - so that exact body supplies the
 * statement order, operand forms, offset-local spelling and declaration style
 * while this body writes its own +0x2A literal. Because that word differs the
 * range is not byte-identical and the clone lever does not apply. The nearer
 * exact sibling
 * seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area17213_801F3E90
 * of emi/world04/area172/13 at 0x801F3E90 repeats the same statement sequence
 * with its corner offsets at +0x18 and -0x18, a (rand() & 0x3) + 0x4 scale and
 * a cleared +0x2A, so it is shape evidence only.
 *
 * Call evidence: the only reference to 0x801F4394 in this payload is the jal at
 * 0x801F355C inside the 0x10-iteration walk of the exact progress handler
 * retreatWorkExtentAndAdvanceOriginZDrawThenOnCountdownExpiryArm20ReseedEffectEntriesQueueCue215AndAdvanceWorkByte1World04Area18813_801F3484,
 * which advances one byte pointer by 0x2C per iteration and passes it in $a0,
 * so the argument is that table entry. The table base is 0x800E48C0, the same
 * 0x2C-byte-stride effect table the exact neighbour
 * drawEffectEntriesThenAdvanceAngle24By10World04Area18813_801F44B4 walks with
 * this same counter and cursor shape, and the exact same-target tables claim
 * 0x801F40F4 as that table draw pass.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area18813_801F4394(
    u8* entry) {
  u8* work;
  MATRIX matrix;
  s16 angle[3];

  work = D_1F800044;
  FIELD_REF(u32, entry, 0x00u) = FIELD_REF(u32, work, 0x34u);
  FIELD_REF(u32, entry, 0x04u) = FIELD_REF(u32, work, 0x38u);
  FIELD_REF(u32, entry, 0x08u) = FIELD_REF(u32, work, 0x3cu);

  angle[0] = (s16)(rand() & 0xfff);
  angle[1] = (s16)((rand() & 0x7ff) - 0x400);
  angle[2] = (s16)(rand() & 0xfff);

  FIELD_REF(s16, entry, 0x10u) = (s16)func_801783C8(0x10);
  FIELD_REF(s16, entry, 0x12u) = (s16)func_801782FC(0x10);
  FIELD_REF(s16, entry, 0x14u) = 0;
  FIELD_REF(s16, entry, 0x18u) = (s16)func_801783C8(-0x10);
  FIELD_REF(s16, entry, 0x1au) = (s16)func_801782FC(-0x10);
  FIELD_REF(s16, entry, 0x1cu) = 0;

  func_801AFF64(&matrix);
  func_80179C58(angle[0], &matrix);
  func_80179DF8(-angle[1], &matrix);
  func_80179F98(angle[2], &matrix);

  ApplyMatrixSV(&matrix, (SVECTOR*)(entry + 0x10), (SVECTOR*)(entry + 0x10));
  ApplyMatrixSV(&matrix, (SVECTOR*)(entry + 0x18), (SVECTOR*)(entry + 0x18));

  FIELD_REF(s16, entry, 0x28u) = (s16)((rand() & 0x2) + 0x3);
  FIELD_REF(s16, entry, 0x20u) = 0;
  FIELD_REF(s16, entry, 0x22u) = 0;
  FIELD_REF(s16, entry, 0x24u) = 0;
  FIELD_REF(s16, entry, 0x2au) = 0x20;
}
