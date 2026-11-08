#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared EXE helpers this entry draw pass reaches: the engine primitive stamper
 * 0x8017A97C, the sine/cosine pair 0x801782FC and 0x801783C8 (1000h per turn,
 * the pair the spin-angle halfword at entry byte 0x24 feeds) and the projection
 * helper 0x801AFF04, which projects one three-word point into a vertex pair.
 * Declared with the same prototypes the sibling lifters of this payload use. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_8017A97C(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801F3BFC
 * @behavior Effect entry draw pass run by 0x801F3FAC for every one of the 0x20
 *           entries of the 0x2C-byte-stride effect table at 0x800E48C0, with the
 *           entry base as its only argument and no result. It stamps the shared
 *           primitive cursor g_PrimCursor through 0x8017A97C, turns
 *           semi-transparency on, projects the entry's own three-word position at
 *           bytes 0x00/0x04/0x08 into the first vertex of the POLY_G3 at cursor
 *           byte 0x08 through 0x801AFF04, and then builds the two remaining
 *           vertices at cursor bytes 0x10 and 0x18. Each of those rotates one
 *           signed-halfword extent pair by the spin angle halfword at entry byte
 *           0x24 - cosine times the first component minus sine times the second
 *           for x, sine times the first plus cosine times the second for y, both
 *           shifted right by 8 and multiplied by the entry scale halfword at
 *           0x28 - while z is the halfword at 0x14 (or 0x1C for the last vertex)
 *           shifted left by 12 and multiplied by the same scale; the three words
 *           are then translated by the entry position before being projected.
 *           Finally it tints all three channels of the first vertex with the
 *           entry halfword at 0x2A clamped to 0..0xFF, zeroes the second and
 *           third vertices' channels, and appends the 0x1C-byte primitive
 *           through 0x8014E5A0 for ordering-table index 1.
 *
 * Evidence from the original bytes at 0x801F3BFC: the 0x38-byte frame with $ra
 * at 0x34, $s2 at 0x30, $s1 at 0x2C and $s0 at 0x28; the g_PrimCursor load into
 * $s2 and the entry copy into $s0; jal 0x8017A97C with $a0 already the cursor;
 * jal SetSemiTrans with addiu $a1,$zero,0x1 in its delay slot; then jal
 * 0x801AFF04 with addiu $a1,$s2,0x8, so the entry itself is the source point of
 * the first vertex. Both rotation blocks open with the same call pair - jal
 * 0x801783C8 then jal 0x801782FC for x, jal 0x801782FC then jal 0x801783C8 for
 * y - each reloading lh $a0,0x24($s0) immediately before the call, with the
 * addu $s1,$v0,$zero in the second call's delay slot carrying the first result.
 * The products are mult/mflo with sra 8 and the scale multiply before the three
 * sw into the stack point at 0x10/0x14/0x18, and each component is then added to
 * the matching lw from the entry at 0x00/0x04/0x08. The clamp is lh $v0,0x2A($s0)
 * with bgez and slti $v0,$v0,0x100 selecting 0, the value itself or 0xFF into
 * $v1, followed by sb $v1 to cursor bytes 0x4/0x5/0x6, six sb $zero to bytes
 * 0xC/0xD/0xE and 0x14/0x15/0x16, and jal 0x8014E5A0 with addiu $a1,$zero,0x1C
 * in its delay slot.
 *
 * Shape evidence: the exact sibling func_801F3944 at
 * emi/world00/area024/14@0x801F3944 repeats the whole body - the same stamper and
 * SetSemiTrans prologue, the same three projected vertices at cursor 0x08/0x10/
 * 0x18 from the entry's 0x10 and 0x18 extent pairs, the same >> 8 and << 12
 * scaling by the 0x28 factor, the same clamp of the 0x2A halfword and the same
 * 0x1C-byte append - and differs only in writing half of the clamped value into
 * the blue channel, so it is shape evidence and not an eligible clone: the 660
 * bytes at 0x801F3BFC against 660 at 0x801F3944 differ in the 28 words from
 * 0x801F3E1C on, where the sibling inserts srl $v0,$v0,1 before its blue store.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudTriangleFromWorkEntryWorld04Area17213_801F3BFC(
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
  primitive->b0 = color;
  primitive->r1 = 0;
  primitive->g1 = 0;
  primitive->b1 = 0;
  primitive->r2 = 0;
  primitive->g2 = 0;
  primitive->b2 = 0;
  GpuAppendPrim(1u, 0x1cu);
}
