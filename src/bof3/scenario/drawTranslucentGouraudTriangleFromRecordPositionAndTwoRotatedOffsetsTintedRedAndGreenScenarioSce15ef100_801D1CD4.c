#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared EXE helpers this record draw pass reaches: the engine primitive
 * stamper 0x8017A97C, the sine/cosine pair 0x801782FC and 0x801783C8 (1000h per
 * turn, the pair the spin-angle halfword at record byte 0x24 feeds) and the
 * projection helper 0x801AFF04, which projects one three-word point into a
 * vertex pair. Declared with the same prototypes the sibling lifters of this
 * payload use. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801D1CD4
 * @behavior Draw pass for one 0x2C-byte effect record of this overlay, the
 *           helper the record walker 0x801D2088 calls for each of the 0x20
 *           records of the table at 0x801D3B28 while it adds 0x10 to the spin
 *           angle of the record it just drew. It takes the record base as its
 *           only argument and returns nothing. It stamps the shared primitive
 *           cursor g_PrimCursor through 0x8017A97C, turns semi-transparency on
 *           and projects the record position triple at +0x0/+0x4/+0x8 into the
 *           first vertex of the POLY_G3 at cursor byte 0x08 through 0x801AFF04,
 *           then builds the two further vertices at cursor bytes 0x10 and 0x18.
 *           Each of those rotates one signed-halfword extent pair by the spin
 *           angle halfword at record byte 0x24 - cosine times the first
 *           component minus sine times the second for x, sine times the first
 *           plus cosine times the second for y, both shifted right by 8 and
 *           multiplied by the scale halfword at 0x28 - while z is the halfword
 *           at 0x14 for the first vertex and 0x1C for the second, shifted left
 *           by 12 and multiplied by the same scale; the three words are then
 *           translated by the record position before each projection. Finally
 *           it writes the halfword at 0x2A clamped to 0..0xFF into the red and
 *           green channels of the first vertex, leaves that vertex blue channel
 *           zero, zeroes the remaining channels of the second and third
 *           vertices, and appends the 0x1C-byte primitive through 0x8014E5A0
 *           for ordering-table index 2.
 *
 * Evidence from the original bytes at 0x801D1CD4: the 0x38-byte frame with $ra
 * at 0x34, $s2 at 0x30, $s1 at 0x2C and $s0 at 0x28; the g_PrimCursor load into
 * $s2 before the record copy into $s0; jal 0x8017A97C with $a0 already the
 * cursor; jal SetSemiTrans with addiu $a1,$zero,0x1 in its delay slot; then jal
 * 0x801AFF04 with addiu $a1,$s2,0x8, so the record itself is the source point
 * of the first vertex. Both rotation blocks open with the same call pair - jal
 * 0x801783C8 then jal 0x801782FC for x, jal 0x801782FC then jal 0x801783C8 for
 * y - each reloading lh $a0,0x24($s0) immediately before the call and carrying
 * the first result through addu $s1,$v0,$zero in the second call delay slot.
 * The products are mult/mflo with sra 8 and the scale multiply before the three
 * sw into the stack point at 0x10/0x14/0x18, and each component is then added
 * to the matching lw from the record at 0x00/0x04/0x08. The clamp is
 * lh $v0,0x2A($s0) with bgez and slti $v0,$v0,0x100 selecting 0, the value
 * itself or 0xFF into $v1, followed by sb $v1 to cursor bytes 0x4 and 0x5,
 * sb $zero to cursor byte 0x6, six sb $zero to bytes 0xC/0xD/0xE and
 * 0x14/0x15/0x16, and jal 0x8014E5A0 reached with addiu $a0,$zero,0x2 and
 * addiu $a1,$zero,0x1C.
 *
 * Shape evidence: the exact sibling func_801F3BFC at
 * emi/world04/area172/13@0x801F3BFC repeats this whole body over the same
 * 0x2C-byte effect records and its 660 bytes against the 660 here differ in
 * exactly four words - the internal jump at 0x801D1F04, which shifts by the
 * 0x1F28 address delta between the two spans, the two addiu $a0,$zero words of
 * the append call, which carry the ordering-table index 1 there against 2 here,
 * and the sb of cursor byte 0x6, which the sibling fills with the clamped value
 * and this span with $zero. The stored literals are this payload own, so the
 * sibling supplied the statement order, operand forms and guard topology only;
 * a byte-identical twin does not exist, since the 660 bytes at 0x801D1CD4 match
 * no other payload span.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudTriangleFromRecordPositionAndTwoRotatedOffsetsTintedRedAndGreenScenarioSce15ef100_801D1CD4(
    u8* record) {
  POLY_G3* primitive;
  VECTOR point;
  s16 shade;
  u8 color;

  primitive = (POLY_G3*)g_PrimCursor;
  func_8017A97C(primitive);
  SetSemiTrans(primitive, 1);
  func_801AFF04(record, &primitive->x0);

  point.vx = ((func_801783C8(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x10u) -
               func_801782FC(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x12u)) >>
              8) *
             FIELD_REF(s16, record, 0x28u);
  point.vy = ((func_801782FC(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x10u) +
               func_801783C8(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x12u)) >>
              8) *
             FIELD_REF(s16, record, 0x28u);
  point.vz = (FIELD_REF(s16, record, 0x14u) << 12) * FIELD_REF(s16, record, 0x28u);
  point.vx += FIELD_REF(u32, record, 0x00u);
  point.vy += FIELD_REF(u32, record, 0x04u);
  point.vz += FIELD_REF(u32, record, 0x08u);
  func_801AFF04(&point, &primitive->x1);

  point.vx = ((func_801783C8(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x18u) -
               func_801782FC(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x1au)) >>
              8) *
             FIELD_REF(s16, record, 0x28u);
  point.vy = ((func_801782FC(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x18u) +
               func_801783C8(FIELD_REF(s16, record, 0x24u)) *
                   FIELD_REF(s16, record, 0x1au)) >>
              8) *
             FIELD_REF(s16, record, 0x28u);
  point.vz = (FIELD_REF(s16, record, 0x1cu) << 12) * FIELD_REF(s16, record, 0x28u);
  point.vx += FIELD_REF(u32, record, 0x00u);
  point.vy += FIELD_REF(u32, record, 0x04u);
  point.vz += FIELD_REF(u32, record, 0x08u);
  func_801AFF04(&point, &primitive->x2);

  shade = FIELD_REF(s16, record, 0x2au);
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
  primitive->b0 = 0;
  primitive->r1 = 0;
  primitive->g1 = 0;
  primitive->b1 = 0;
  primitive->r2 = 0;
  primitive->g2 = 0;
  primitive->b2 = 0;
  GpuAppendPrim(2u, 0x1cu);
}
