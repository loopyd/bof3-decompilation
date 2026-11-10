#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared phase samplers: both return the sine and cosine of a 12-bit phase in
 * the same fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic siblings give their own seed
 * helper. */
void func_8017A97C(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Scratchpad tint cell at 0x1F80000A, published once before the loop and re-read
 * through the loop's radius base pointer. The original takes this address into
 * a callee-saved register before the first sampler call and derives the radius
 * base from it by subtracting five halfwords, so the cell is a live address and
 * not a folded constant. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Scratchpad next-sample cell at 0x1F800024, the x halfword this body writes
 * before entering the loop and then carries forward each pass. The original
 * forms this address into $s0 with its own `lui 0x1F80` plus `addiu` pair before
 * the first sampler call and later derives the vertex triplet base by
 * subtracting 0x10 from that register, so the triplet is reached through this
 * cell rather than through a symbol at its own first vertex. Its y neighbour at
 * 0x1F800026 and the three z halfwords at 0x1F800028, 0x1F800020 and 0x1F800018
 * are published through literal constants instead, the absolute `lui $at` plus
 * `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Scratchpad radius cell at 0x1F800000, the value every radial product of this
 * body scales its phase samples by. The original publishes it with the absolute
 * `lui $at` plus `sh` form rather than through a pointer, so the source stores
 * through the cell itself, while the loop reads it as a signed halfword through
 * the base pointer derived from the tint address. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* @source 0x801EF68C
 * @behavior Translucent radial fan emitter of this overlay draw group, entered
 *           as the second call of the draw handler at 0x801EF524 between its
 *           own PushMatrix at 0x801EF430 and its PopMatrix, so it publishes
 *           geometry in that handler's local matrix. It first picks the
 *           draw-mode texture page, 0x125 when GetGraphType returns 1 or 2 and
 *           0x55 otherwise, calling it once per test with no cached result,
 *           arms SetDrawMode on the primitive cursor g_PrimCursor with dfe 0,
 *           dtd 1 and a null RECT, and links that 0xC-byte packet under
 *           ordering-table slot 5.
 *           It then reads the scratchpad work-object pointer cell at
 *           0x1F800044 once and publishes the numbers the fan runs on: the
 *           work-object unsigned byte 0x9 times twelve into the single tint
 *           cell 0x1F80000A and 0xF0 into the scratchpad radius cell
 *           0x1F800000. Byte 0x9 is read once for that one product, so the whole
 *           fan is tinted by the same grey level and the cell is not re-read
 *           while the product is formed. The radius store reaches the emitted
 *           order only after the tint product is formed: the compiler moves the
 *           0xF0 store ahead of the multiply.
 *           The next scratchpad sample is seeded at phase zero: the x halfword
 *           0x1F800024 takes func_801782FC(0) times the signed radius shifted
 *           right twelve and its y neighbour 0x1F800026 takes
 *           func_801783C8(0) times the same radius shifted right twelve. The
 *           two reads of the radius cell are taken independently, one in the
 *           absolute fixed-address form and one through the cell symbol, and
 *           neither goes through the loop's base pointer, which the original
 *           derives from the tint address only after both samples.
 *           The loop runs sixteen passes with the phase advancing by 0x100 each
 *           time, so the sixteen wedges span the whole 0x1000 phase circle at
 *           one radius and form a full disc. Each pass takes a fresh
 *           g_PrimCursor primitive, clears the origin vertex pair, carries the
 *           previous pass sample pair into the second vertex, and fills the
 *           third vertex pair with the func_801782FC and func_801783C8 samples
 *           of the current phase scaled by the signed radius and shifted right
 *           twelve. It then clears the depth of all three vertices, seeds the
 *           primitive through func_8017A97C, marks it semi-transparent,
 *           projects the three vertices through RotTransPers3 into the
 *           primitive vertex pairs at 0x08, 0x10 and 0x18, and gives the
 *           primitive the tint cell in the red, green and blue bytes of its
 *           first vertex with 1 in the same three bytes of the second and the
 *           third, then links it as the 0x1C-byte POLY_G3 size under slot 5.
 *           The three tint reads are three separate unsigned halfword loads of
 *           the same cell because the byte stores to the primitive may alias
 *           it, so the origin vertex is re-read before each channel store.
 *           After the loop the body selects the draw-mode texture page again,
 *           this time 0x25 when GetGraphType returns 1 or 2 and 0x15 otherwise,
 *           re-arms SetDrawMode the same way and links a second 0xC-byte packet
 *           under slot 5, then returns.
 *           The 0x40-byte frame keeps $ra, $s4, $s3, $s2, $s1 and $s0 plus the
 *           fifth SetDrawMode argument at 0x10, which the same slot doubles as
 *           the first of the four RotTransPers3 stack arguments at 0x10 to
 *           0x1C and the sampler outputs at 0x20 and 0x24.
 *           The roles of work-object byte 0x9 and of the radius beyond this
 *           published geometry are not established, so they stay
 *           offset-anchored, and the scratchpad cells are named by address.
 *           Shape evidence, not clone proof: no payload under out/binaries holds
 *           this 628-byte span, and the nearest payload body, the radial fan at
 *           0x801F0790 of bmagic/magic006/03, carries its own texture page
 *           constants and radius cell, so it is a shape donor at most. The
 *           statement order and the scratchpad access forms follow the exact
 *           bmagic siblings
 *           drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100
 *           TintedByWorkByte9WeightedEightTwoOne at 0x801F0790 and
 *           drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetime at
 *           0x801EFA5C.
 * Live evidence: 157 of 157 instructions and 628 of 628 bytes, at the original
 *           span size with no differing word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius240TintedByWorkByte9TimesTwelveAtOriginAndOneAtRimBmagicMagic08803_801EF68C(
    void) {
  u8 *work;
  u16 *tint;
  u16 *next;
  u16 *cells;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);

  work = SPAD_PTR_SLOT(u8, 0x44u);
  tint = &D_1F80000A;
  *tint = work[9] * 12;
  D_1F800000 = 0xF0;

  next = &D_1F800024;
  *next = (func_801782FC(0) * SPAD_REF(s16, 0x00u)) >> 12;
  SPAD_REF(u16, 0x26u) = (func_801783C8(0) * *(s16 *)&D_1F800000) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  cells = tint - 5;
  do {
    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)cells) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)cells) >> 12;
    prim = (POLY_G3 *)g_PrimCursor;
    shape[2].vz = 0;
    shape[1].vz = 0;
    shape[0].vz = 0;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
    prim->r0 = cells[5];
    prim->g0 = cells[5];
    prim->b0 = cells[5];
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 0x11);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);
}
