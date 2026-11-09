#include "bof3/bof3.h"
#include <rand.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original materialises it once with a `lui 0x1F80` plus a load at
 * displacement 0x44 ahead of the frame setup and reuses that one base for the
 * three position-word loads, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_801AFF64(void* arg0);
void func_80179C58(s32 arg0, MATRIX* arg1);
void func_80179DF8(s32 arg0, MATRIX* arg1);
void func_80179F98(s32 arg0, MATRIX* arg1);

/* @source 0x801D1F68
 * @behavior Effect-record seeder of the 0x2C-byte-stride record tables of this
 *           payload. It seeds one record from the scratchpad work object behind
 *           0x1F800044: it copies the object's position triple at +0x34, +0x38
 *           and +0x3C into the record words at +0x00, +0x04 and +0x08, fills
 *           the record corner offset SVECTORs at +0x10 and +0x18 from the shared
 *           angular pair 0x801783C8/0x801782FC at +0x10 and -0x10 with their
 *           third halfwords cleared, takes three random Euler angles
 *           (rand() & 0xFFF, (rand() & 0x7FF) - 0x400 and rand() & 0xFFF),
 *           builds the scratch rotation matrix through 0x801AFF64 and the three
 *           shared rotation helpers 0x80179C58, 0x80179DF8 with the middle
 *           angle negated and 0x80179F98, and rotates both corner offset
 *           SVECTORs through that matrix in place with ApplyMatrixSV. It then
 *           arms the record scale halfword at +0x28 to (rand() & 2) + 3, clears
 *           the spin angle at +0x24 together with the halfwords at +0x20 and
 *           +0x22, and writes the constant 0x40 into the second scale halfword
 *           at +0x2A. The result is the record the draw helper 0x801D1CD4 reads
 *           as the quad base at +0x00, the extents at +0x10 and the spin angle
 *           at +0x24.
 *
 * Evidence from the original bytes at 0x801D1F68: `lui $v1,0x1F80` plus
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
 * addiu $v0,$zero,0x40, then sh $zero to 0x20($s0), 0x22($s0) and 0x24($s0)
 * and sh $v0,0x2A($s0).
 *
 * Name and call evidence: the only in-image references are the two in-payload
 * jal sites of the step handlers - 0x801D0EB4 in
 * advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8
 * walking the 0x20 records of 0x801D3B28 and 0x801D11F8 in
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * walking the 0x20 records of 0x801D3DE8, both stepping the record pointer by
 * 0x2C - and both hand over a pointer only, so the record kind is the effect
 * record func_801D1CD4 draws.
 *
 * Shape evidence: the world00 area024/14 seeder
 * seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area17213_801F3E90
 * repeats this whole body - the same scratchpad position copy, the same offset
 * vectors at +0x10 and +0x18 from 0x801783C8/0x801782FC, the same
 * 0x801AFF64/0x80179C58/0x80179DF8/0x80179F98 matrix build, the same in-place
 * ApplyMatrixSV pair and the same cleared 0x20/0x22/0x24 halfwords - and
 * differs in its offset angle 0x18 instead of 0x10, its +0x28 factor
 * (rand() & 3) + 4 instead of (rand() & 2) + 3 and its cleared +0x2A instead of
 * the constant 0x40, so it is shape evidence and not an eligible clone.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68(
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
  FIELD_REF(s16, entry, 0x2au) = 0x40;
}
