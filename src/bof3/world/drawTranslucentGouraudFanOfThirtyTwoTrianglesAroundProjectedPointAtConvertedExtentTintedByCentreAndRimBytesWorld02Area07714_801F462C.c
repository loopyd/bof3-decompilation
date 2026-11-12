#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared frame counter of the resident main executable, bound by the generated
 * undefined_syms_auto row. */
extern s32 D_80143E6C;

/* 0x801AFF04 - shared projector: writes the projected halfword pair of the
 * three-word point handed as arg0 into arg1. */
void func_801AFF04(const void* arg0, void* arg1);

/* 0x801AFFD8 - shared size-pair converter: converts the halfword pair handed
 * as arg1 at the depth of the same point and writes the result into arg2. */
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* 0x801783C8 and 0x801782FC - shared 12-bit angular helpers, the x and the y
 * companion of the rotation pair the sibling fan drawer
 * drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundProjectedWorkRecordPositionAtConvertedExtentTintedByCentreAndRimBytesWorld04Area18813_801F3F30
 * calls for every ring angle. */
s32 func_801783C8(s32 arg0);
s32 func_801782FC(s32 arg0);

/* 0x8017A97C - packet opener: the sibling fan drawers of this family call it
 * on the primitive cursor before they fill a POLY_G3. */
void func_8017A97C(void* arg0);

/* One 8-byte screen entry of this body's projection scratch: the projector and
 * the size-pair converter write the halfword pair at +0x00/+0x02 and this body
 * reads back only that pair, so the trailing four bytes stay unmodelled. */
typedef struct Area07714ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} Area07714ScreenPoint;

/* @source 0x801F462C
 * @behavior Translucent arc fan around one projected point: the caller hands it
 * a three-word point, a signed 16-bit extent and the two colour bytes the
 * resident arc-fan wrappers of this tree hand their fan drawer, and it stamps a
 * full turn of translucent Gouraud wedges around that point's projection.
 * It first projects the point through func_801AFF04 into the first scratch
 * entry, writes the extent into the x halfword of the second entry as the size
 * pair func_801AFFD8 expects, and converts that pair at the same depth into the
 * third entry. The fan radius is the converted halfword plus the low bit of the
 * shared frame counter D_80143E6C, so the disc breathes by one sub-pixel unit on
 * alternating frames, and it is taken as a signed halfword because the rotation
 * products below are signed multiplies.
 *
 * The loop emits thirty-two wedges of one semi-transparent POLY_G3 each. Its
 * phase is an unsigned halfword accumulator that starts at zero and advances by
 * 0x80 per wedge, and the advanced phase it draws the second arc vertex at is
 * that same accumulator masked to the twelve bits of the 0x1000-unit turn, so
 * the thirty-two wedges close the disc exactly one turn after the first. Every
 * wedge takes the projected point as vertex 0, the point plus the radius rotated
 * by the phase as vertex 1 through func_801783C8 for x and func_801782FC for y
 * with both products shifted down by twelve, and the point plus the radius
 * rotated by the already advanced phase as vertex 2, so consecutive wedges share
 * an arc edge. The first colour triplet takes the centre colour byte in all
 * three channels and the second and third take the rim byte, so the caller's
 * drawFour pass lights a 0x20 hub inside a black rim. Each packet is appended
 * under ordering table slot 1 with the 0x1C-byte POLY_G3 size.
 *
 * Evidence: payload asm at 0x801F462C - addiu $sp,$sp,-0x48 with sw
 * $s0,0x28, sw $s1,0x2C, sw $s6,0x40, sw $s4,0x38, sw $ra,0x44, sw $s5,0x3C,
 * sw $s3,0x34 and sw $s2,0x30, the argument copies addu $s0,$a0 / addu $s1,$a1
 * / addu $s6,$a2 / addu $s4,$a3, then addiu $a1,$sp,0x10 and jal 0x801AFF04
 * with sw $s2,0x30 in its delay slot, addiu $a1,$sp,0x18 and addiu $a2,$sp,0x20
 * with jal 0x801AFFD8 and sh $s1,0x18 in its delay slot, so the extent enters the
 * size pair rather than the phase. The radius block is addu $s1,$zero,$zero, the
 * lui/lw pair of 0x80143E6C, lhu $v0,0x20($sp), andi $v1,$v1,0x1, addu, sll 0x10
 * and sra $s5,$v0,0x10 - the zeroing of the step accumulator proves the fan
 * starts at phase zero whatever the caller passes. The loop is lui/lw of
 * g_PrimCursor into $s0, jal 0x8017A97C with addu $a0,$s0, jal SetSemiTrans with
 * addiu $a1,$zero,0x1, then addiu $s3,$s1,0x80, andi $s2,$s3,0xFFF and
 * andi $s1,$s1,0xFFFF before the coordinate block, lhu 0x10($sp) and
 * sh 0x8($s0), lhu 0x12($sp) and sh 0xA($s0), then the four
 * jal 0x801783C8 / jal 0x801782FC pairs each followed by mult $s5,$v0, mflo,
 * sra $v1,$t0,0xC and addu onto a fresh lhu of 0x10($sp) or 0x12($sp), with
 * andi $s2,$s2,0xFFFF before the second pair and addu $s1,$s3,$zero between the
 * second and the third pair, and the stores at 0x10/0x12 and 0x18/0x1A folded
 * into the two jal delay slots. The colour block is sb $s6,0x4/0x5/0x6 and
 * sb $s4,0xC/0xD/0xE/0x14/0x15/0x16, one natural triplet per vertex, and
 * jal 0x8014E5A0 carries addiu $a0,$zero,0x1 with addiu $a1,$zero,0x1C while the
 * last rotated halfword lands in its delay slot as sh $v1,0x1A($s0). The back
 * edge is andi $v0,$s1,0xFFFF, sltiu $v0,$v0,0x1000 and bnez to the loop head,
 * so the loop is one unsigned halfword phase bound, and the epilogue reloads
 * $ra, $s6, $s5, $s4, $s3, $s2, $s1 and $s0 and returns with jr $ra.
 *
 * Frame: the 0x48-byte frame carries three 8-byte screen entries from sp+0x10 to
 * sp+0x27 and the saved group $s0..$s6 plus $ra from sp+0x28 to sp+0x47, so one
 * address-taken array of that entry type keeps the three entries together across
 * the calls.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundProjectedPointAtConvertedExtentTintedByCentreAndRimBytesWorld02Area07714_801F462C(
    const void* point, s16 extent, u8 centre_color, u8 rim_color) {
  Area07714ScreenPoint screen[3];
  POLY_G3*             prim;
  s16                  radius;
  u16                  phase;
  u16                  next;

  func_801AFF04(point, &screen[0]);
  screen[1].x = extent;
  func_801AFFD8(point, &screen[1], &screen[2]);

  radius = (s16)(screen[2].x + (D_80143E6C & 1));
  phase = 0;

  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    next = (phase + 0x80) & 0xFFF;
    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = (u16)(screen[0].x + ((radius * func_801783C8(phase)) >> 12));
    prim->y1 = (u16)(screen[0].y + ((radius * func_801782FC(phase)) >> 12));
    phase += 0x80;
    prim->x2 = (u16)(screen[0].x + ((radius * func_801783C8(next)) >> 12));
    prim->y2 = (u16)(screen[0].y + ((radius * func_801782FC(next)) >> 12));
    prim->r0 = centre_color;
    prim->g0 = centre_color;
    prim->b0 = centre_color;
    prim->r1 = rim_color;
    prim->g1 = rim_color;
    prim->b1 = rim_color;
    prim->r2 = rim_color;
    prim->g2 = rim_color;
    prim->b2 = rim_color;
    GpuAppendPrim(1u, 0x1Cu);
  } while (phase < 0x1000);
}
