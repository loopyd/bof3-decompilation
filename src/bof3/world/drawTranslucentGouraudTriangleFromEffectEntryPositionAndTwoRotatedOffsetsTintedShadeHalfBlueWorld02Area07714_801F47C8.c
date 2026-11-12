#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared EXE helpers this draw pass reaches: the engine primitive stamper
 * 0x8017A97C, the sine/cosine pair 0x801782FC and 0x801783C8 that the spin
 * halfword at entry byte 0x24 feeds, and the projection helper 0x801AFF04,
 * which projects one three-word point into the vertex pair written through its
 * second argument. Declared with the prototypes this payload's sibling lifters
 * use. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801F47C8
 * @behavior Draw pass for one 0x2C-byte effect entry of this overlay, the
 *           helper the overlay draw pass 0x801F3F08 calls once for each of the
 *           0x20 entries of the 0x2C-byte-stride effect table at 0x800E5504,
 *           between advancing that entry's spin halfword +0x24 by 0x10 and
 *           stepping its shade halfword +0x2A by 0x20 or down by 2. It takes
 *           the entry base as its only argument and returns nothing. It stamps
 *           the shared primitive cursor g_PrimCursor through 0x8017A97C, turns
 *           semi-transparency on and projects the entry position triple at
 *           +0x0/+0x4/+0x8 into the first vertex of the POLY_G3 at cursor byte
 *           0x08 through 0x801AFF04, then builds the two further vertices at
 *           cursor bytes 0x10 and 0x18. Each of those rotates one signed-halfword
 *           extent pair by the spin angle halfword at entry byte 0x24 - cosine
 *           times the first component minus sine times the second for x, sine
 *           times the first plus cosine times the second for y, both shifted
 *           right by 8 and multiplied by the scale halfword at 0x28 - while z is
 *           the halfword at 0x14 for the first vertex and 0x1C for the second,
 *           shifted left by 12 and multiplied by the same scale; the three words
 *           are then translated by the entry position before each projection.
 *           Finally it reads the shade halfword at 0x2A, clamps it to 0..0xFF,
 *           writes that value into the red and green channels of the first
 *           vertex, writes half of it into that vertex's blue channel, zeroes
 *           the colour bytes of the second and third vertices at cursor bytes
 *           0xC/0xD/0xE and 0x14/0x15/0x16, and appends the 0x1C-byte primitive
 *           through 0x8014E5A0 for ordering-table index 1.
 *
 * Evidence from the original bytes at 0x801F47C8: the 0x38-byte frame with $ra
 * at 0x34, $s2 at 0x30, $s1 at 0x2C and $s0 at 0x28; the g_PrimCursor lui/lw
 * pair into $s2 taken before the entry copy into $s0 and kept for the whole
 * body, so every cursor access reuses that register; jal 0x8017A97C with $a0
 * already the cursor; jal SetSemiTrans with addiu $a1,$zero,0x1 in its delay
 * slot; then jal 0x801AFF04 with addiu $a1,$s2,0x8, so the entry itself is the
 * source point of the first vertex. Both rotation blocks open with the same call
 * pair - jal 0x801783C8 then jal 0x801782FC for x, jal 0x801782FC then jal
 * 0x801783C8 for y - each reloading lh $a0,0x24($s0) immediately before the
 * call and carrying the first result through addu $s1,$v0,$zero in the second
 * call delay slot. The products are mult/mflo with sra 8 and the scale multiply
 * before the three sw into the stack point at 0x10/0x14/0x18, and each component
 * is then added to the matching lw from the entry at 0x00/0x04/0x08. The clamp
 * is lh $a0,0x2A($s0) with bgez to its own block, addu $v1,$v0,$zero and
 * slti $v0,$a0,0x100 selecting the value itself, 0 or 0xFF, with the
 * addiu $a0,$zero,0x1 of the append index already loaded in the branch delay
 * slot; then andi/srl of that value by one, sb to cursor bytes 0x4 and 0x5,
 * sb of the halved value to cursor byte 0x6, six sb $zero to bytes 0xC/0xD/0xE
 * and 0x14/0x15/0x16, and jal 0x8014E5A0 reached with addiu $a0,$zero,0x1 and
 * addiu $a1,$zero,0x1C.
 *
 * Shape evidence: the exact sibling
 * drawTranslucentGouraudTriangleFromEffectEntryPositionAndTwoRotatedOffsetsTintedShadeHalfBlueWorld03Area13013_801F2E30
 * at emi/world03/area130/13@0x801F2E30 repeats this whole 0x2A0-byte body over
 * the same 0x2C-byte effect entries and carries the same ordering-table index 1.
 * The byte comparison cmp -l -i 0x1BC8:0x230 -n 0x2A0 of the two payloads
 * reports only bytes 561 and 562 of the span, the low half of the internal j at
 * 0x801F49F8, whose absolute target shifts by the 0x1998 address delta between
 * the two spans. The sibling supplied the statement order, operand forms and
 * declaration style; this span's own literals were written from its bytes. A
 * scan of every payload under out/binaries finds no byte-identical 0x2A0 span:
 * the nearest repeats stop at the relocated j word, so the range has no clone
 * twin.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudTriangleFromEffectEntryPositionAndTwoRotatedOffsetsTintedShadeHalfBlueWorld02Area07714_801F47C8(
    u8* entry) {
  POLY_G3* primitive;
  VECTOR point;
  s16 shade;
  u8 color;

  primitive = (POLY_G3*)g_PrimCursor;
  func_8017A97C(primitive);
  SetSemiTrans(primitive, 1);
  func_801AFF04(entry, &primitive->x0);

  point.vx = ((func_801783C8(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x10u) -
               func_801782FC(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x12u)) >>
              8) *
             FIELD_REF(s16, entry, 0x28u);
  point.vy = ((func_801782FC(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x10u) +
               func_801783C8(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x12u)) >>
              8) *
             FIELD_REF(s16, entry, 0x28u);
  point.vz = (FIELD_REF(s16, entry, 0x14u) << 12) * FIELD_REF(s16, entry, 0x28u);
  point.vx += FIELD_REF(u32, entry, 0x00u);
  point.vy += FIELD_REF(u32, entry, 0x04u);
  point.vz += FIELD_REF(u32, entry, 0x08u);
  func_801AFF04(&point, &primitive->x1);

  point.vx = ((func_801783C8(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x18u) -
               func_801782FC(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x1au)) >>
              8) *
             FIELD_REF(s16, entry, 0x28u);
  point.vy = ((func_801782FC(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x18u) +
               func_801783C8(FIELD_REF(s16, entry, 0x24u)) *
                   FIELD_REF(s16, entry, 0x1au)) >>
              8) *
             FIELD_REF(s16, entry, 0x28u);
  point.vz = (FIELD_REF(s16, entry, 0x1cu) << 12) * FIELD_REF(s16, entry, 0x28u);
  point.vx += FIELD_REF(u32, entry, 0x00u);
  point.vy += FIELD_REF(u32, entry, 0x04u);
  point.vz += FIELD_REF(u32, entry, 0x08u);
  func_801AFF04(&point, &primitive->x2);

  shade = FIELD_REF(s16, entry, 0x2au);
  if (shade < 0) {
    color = 0;
  } else {
    color = shade;
    if (shade >= 0x100) {
      color = 0xff;
    }
  }
  primitive->r0 = color;
  primitive->g0 = color;
  primitive->b0 = color >> 1;
  primitive->r1 = 0;
  primitive->g1 = 0;
  primitive->b1 = 0;
  primitive->r2 = 0;
  primitive->g2 = 0;
  primitive->b2 = 0;
  GpuAppendPrim(1u, 0x1cu);
}
