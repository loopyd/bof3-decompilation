#include "bof3/bof3.h"

/* Shared angular pair of the resident maths helpers: 0x801783C8 is the cosine
 * sample and 0x801782FC the sine sample the exact siblings multiply by a radius
 * before shifting it down. */
s32 func_801783C8(s32 arg0);
s32 func_801782FC(s32 arg0);

/* Resident libgte matrix entry points the exact siblings use to build the
 * scratch rotation matrix. */
void func_801AFF64(void* arg0);
void func_80179DF8(s32 arg0, MATRIX* arg1);
void func_80179F98(s32 arg0, MATRIX* arg1);

/* Resident three-word vector transform: it loads the matrix at arg0 and the
 * three-word vector at arg1 and writes the transformed vector at arg2, the
 * schedule that follows the 0x20-byte matrix block and the 16-byte VECTOR pair
 * this body reserves. */
void func_80178A18(MATRIX* arg0, VECTOR* arg1, VECTOR* arg2);

/* @source 0x801F7FFC
 * @behavior Publishes the four 16-byte corner vertex records of this overlay's
 * work object. The received pointer is the work object plus 0x0C, the same
 * pointer both of its in-payload callers hand to 0x801F8170, so the records it
 * writes at +0x0C, +0x10 and +0x14 of each 16-byte slot are the work object's
 * words at 0x18 onward - exactly the four records that
 * drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170
 * fans the two translucent Gouraud triangles over. The body arms one scratch
 * rotation matrix from the object's halfwords 0x5A and 0x5C, then walks four
 * corners in the plane of that matrix: the corner offsets are (0, 0), (x, y),
 * (x, 0) and (x, -y) with x and y the cosine and sine samples of the object's
 * 0x58 angle scaled by its 0x5E halfword and shifted up by 4, and each offset
 * vector is transformed through the matrix and added to the object's position
 * triple at 0x0C/0x10/0x14. The x and y sums are plain word sums while the
 * transformed z is shifted up by 8 before it is added, so the depth axis keeps
 * its own fixed-point scale. The two angle halfwords at 0x5A and 0x5C are the
 * arm-suffix step handlers' own - 0x801F7BB0 adds 4 to 0x58, 0x801F7D80
 * subtracts 8 from it and resets the state once it reads negative - so the point
 * this rebuilds is the spinning effect quad the following emitter draws.
 *
 * Evidence from the original bytes at 0x801F7FFC: `addiu $sp,$sp,-0x68` with
 * sw $s0,0x50($sp), addu $s0,$a0,$zero and sw $ra,0x60($sp); lh $a0,0x4C($s0)
 * ahead of jal 0x801783C8 with addu $s1,$zero,$zero in its delay slot; then
 * lh $v1,0x52($s0), mult $v1,$v0, lh $a0,0x4C($s0), mflo $a3 and jal 0x801782FC
 * with sll $s2,$a3,4 in its delay slot, and the same 0x52/0x4C reload pair
 * around jal 0x801AFF64 with addiu $a0,$sp,0x10 before the mflo and sll $s3,$a3,4
 * in the delay slot. jal 0x80179DF8 with lh $a0,0x4E($s0) and jal 0x80179F98 with
 * lh $a0,0x50($s0) both pass addiu $a1,$sp,0x10, then jal 0x80178A18 takes
 * addiu $a0,$sp,0x10, addiu $a1,$sp,0x30 and addiu $a2,$sp,0x40 before
 * sw $zero,0x38($sp). The loop head .L801F8088 is andi $v1,$s1,0xFF with
 * addiu $v0,$zero,0x1, beq and slti $v0,$v1,2 wiring the four arms
 * (sw $zero,0x30($sp)/sw $zero,0x34($sp), sw $s2/sw $s3, sw $s2/sw $zero and
 * negu $v0,$s3 into sw $s2,0x30($sp)/sw $v0,0x34($sp)), and the .L801F80F0 tail
 * repeats jal 0x80178A18 with the same three stack pointers before andi
 * $a0,$s1,0xFF, sll $a0,$a0,4, addu $a0,$s0,$a0, the lw $v0,0x0($s0)/
 * lw $v1,0x40($sp) sum into sw 0xC($a0), the lw 0x4($s0)/lw 0x44($sp) sum into
 * sw 0x10($a0) with addiu $s1,$s1,1 between them, and lw 0x48($sp) with
 * sll $v0,$v0,8 added to lw 0x8($s0) into sw 0x14($a0) in the bnez delay slot of
 * andi $v0,$s1,0xFF with sltiu $v0,$v0,4.
 *
 * Levers: the data lever is declined because the range opens with
 * `addiu $sp, $sp, -0x68`, ends with `jr $ra` and holds nine `jal` sites; the
 * carve lever is declined because the span is one declared row from this entry
 * to the next declared entry 0x801F8170 with its own prologue and return path;
 * the clone lever is declined because a scan of every payload under
 * out/binaries finds this 0x174-byte span only in its own payload, and only in
 * its own payload with every `jal` word masked to its opcode, which is the
 * conclusive form here because every callee resolves in the shared main
 * executable rather than in this payload.
 *
 * Frame: the 0x68-byte frame holds the 0x20-byte scratch MATRIX at sp+0x10, the
 * 16-byte corner VECTOR at sp+0x30, the 16-byte transformed VECTOR at sp+0x40,
 * $s0/$s1/$s2/$s3 at sp+0x50/0x54/0x58/0x5C and $ra at sp+0x60.
 *
 * Live audit: the duplicated pre-loop 0x80178A18 call is in the original - it
 * runs before the loop head with the same three stack pointers, so the source
 * carries one call ahead of corner.vz = 0 and one at the top of each iteration.
 * The corner scale is written radius-first, `0x5E * sample(0x58)`, which is the
 * order the original's `mult $v1,$v0` fixes, and the four record stores go
 * through the `dst = work + (i << 4)` pointer local: adding the index offset to
 * the work base inside one FIELD_REF measured `addu $a0,$a0,$s0` instead of the
 * original's `addu $a0,$s0,$a0`, and a 4-variant shape sweep over that form,
 * the reversed constant order and the raw `*(s32 *)` spelling left this shape
 * as the only 93/93 candidate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void buildFourRotatedQuadCornerVerticesFromWorkCentreAndAnglesScenarioScena0300_801F7FFC(
    u8* work) {
  MATRIX matrix;
  VECTOR corner;
  VECTOR point;
  s32 offset_x;
  s32 offset_y;
  u8 i;
  u8 *dst;

  offset_x = (FIELD_REF(s16, work, 0x52u) *
              func_801783C8(FIELD_REF(s16, work, 0x4cu))) << 4;
  offset_y = (FIELD_REF(s16, work, 0x52u) *
              func_801782FC(FIELD_REF(s16, work, 0x4cu))) << 4;

  func_801AFF64(&matrix);
  func_80179DF8(FIELD_REF(s16, work, 0x4eu), &matrix);
  func_80179F98(FIELD_REF(s16, work, 0x50u), &matrix);

  func_80178A18(&matrix, &corner, &point);
  corner.vz = 0;

  for (i = 0; i < 4; i++) {
    switch (i) {
      case 0:
        corner.vx = 0;
        corner.vy = 0;
        break;
      case 1:
        corner.vx = offset_x;
        corner.vy = offset_y;
        break;
      case 2:
        corner.vx = offset_x;
        corner.vy = 0;
        break;
      default:
        corner.vx = offset_x;
        corner.vy = -offset_y;
        break;
    }

    func_80178A18(&matrix, &corner, &point);

    dst = work + (i << 4);

    FIELD_REF(s32, dst, 0x0cu) = FIELD_REF(s32, work, 0x00u) + point.vx;
    FIELD_REF(s32, dst, 0x10u) = FIELD_REF(s32, work, 0x04u) + point.vy;
    FIELD_REF(s32, dst, 0x14u) =
        FIELD_REF(s32, work, 0x08u) + (point.vz << 8);
  }
}
