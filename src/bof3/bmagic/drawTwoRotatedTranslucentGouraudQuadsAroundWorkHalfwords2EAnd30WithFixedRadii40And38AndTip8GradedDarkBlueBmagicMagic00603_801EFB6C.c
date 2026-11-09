#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay's exact lifts
 * drawTranslucentGouraudQuadAroundWorkHalfwords2EAnd30WithHalfExtentsFromWork
 * Bytes9AndAGradedBlackToDarkBlueBmagicMagic00603_801F0308 and
 * drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100TintedByWork
 * Byte9WeightedEightTwoOneBmagicMagic00603_801F0790 already use. This body
 * re-forms the cell address and re-loads the pointer at the head of the first
 * quad and again inside each rotation-angle expression of the later vertices,
 * so every read is its own `lui` plus `lw` pair. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad centre cells at 0x1F800000 and 0x1F800002, the x and y halfword
 * this body publishes from the work object's halfwords 0x2E and 0x30 and then
 * re-reads as the base of every vertex expression in both quads. The two
 * corner stores of each quad must each re-read its cell instead of carrying the
 * base from the previous vertex, which is what the volatile qualifier forces;
 * the reads stay unsigned halfwords, so the qualifier adds no sign-extension
 * sequence. The y cell has a map row because the exact sibling above binds it
 * there; the x cell resolves through the target's generated undefined-symbol
 * table. */
extern volatile u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern volatile u16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Scratchpad radius cell at 0x1F800004, published with the fixed tip radius 8
 * before the first quad and read back once per tip vertex. The original forms
 * its address as one `lui $s3` plus `addiu $s3` pair before the first publish
 * and keeps that base in a saved register across both quads, while every other
 * scratchpad cell is re-formed absolutely at each access, so the source keeps
 * one pointer to this cell and addresses it at offset zero. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad radius cell at 0x1F800006, the halfword the body writes with the
 * current radius before each rotated vertex and reads back as the signed `mult`
 * operand of that vertex's two products. It stays unqualified so the just
 * stored radius reaches the first product as a register and only the second
 * product re-loads it with `lh`. */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Scratchpad phase cell at 0x1F800008, the twelve-bit turn the body writes
 * before each rotated vertex. The first product of a vertex takes the stored
 * register, the second re-loads the cell with `lh`, so it stays unqualified for
 * the same reason as the radius cell. The redundant store this body makes into
 * the same halfword before the first quad's tip vertex is issued through a
 * volatile lvalue, which is the only form that keeps it: as a plain store the
 * compiler discards it, and as a volatile access through this symbol the
 * overall allocation of the body changes. */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared 12-bit angular helpers: func_801782FC supplies the x term of a
 * rotated offset and func_801783C8 the y term, both in the same fixed-point
 * scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Shape and naming evidence: this body is the fixed-radius member of the same
 * drawing family as func_801EF478 at 0x801EF478, which walks the shared radius
 * table at 0x801F0A30 in a loop and stays assembly, and of the two exact lifts
 * named above, which publish the same work halfwords 0x2E and 0x30 into the
 * same scratchpad centre cells. Its only twin search covered every payload
 * under out/binaries with the span's relocation words masked and found no
 * byte-identical copy (the closest payload shares 41 of 318 words), so the
 * clone lever had no source and the body was written from the original bytes.
 * The statement order follows the exact bmagic sibling at 0x801F0308, which
 * also selects the 0xA5 or 0x35 texture page, links a 0xC-byte packet first and
 * a 0x24-byte gouraud quad after the vertex and colour stores. */
/* @source 0x801EFB6C
 * @behavior Draw routine of this overlay's work-byte-2 handler group. Its only
 *           in-image reference is the jal at 0x801EEF00 inside func_801EEE50,
 *           taken after the shared helper at 0x800AF764 while the work object's
 *           bytes 0, 2 and 0xB are all non-zero; the same block calls
 *           func_801EF478 instead when byte 0xB is zero, so that byte selects
 *           between the two draw variants. The body reads no incoming register,
 *           so it takes no arguments and returns nothing.
 *           It opens the draw-mode selection: 0xA5 when GetGraphType returns 1
 *           or 2 and 0x35 otherwise, called once per test with no cached
 *           result, SetDrawMode armed at g_PrimCursor with dfe 0, dtd 1 and a
 *           null RECT, and a 0xC-byte packet linked under ordering-table slot
 *           3.
 *           It then publishes the geometry constants into the scratchpad: the
 *           fixed tip radius 8 into the radius cell 0x1F800004, and the work
 *           object's unsigned halfwords 0x2E and 0x30 into the centre cells
 *           0x1F800000 and 0x1F800002. A pointer to the tip cell stays live in
 *           a saved register for the rest of the body.
 *           The body then emits two translucent gouraud quads. Each one takes
 *           a fresh g_PrimCursor primitive, tags it through SetPolyG4, marks it
 *           semi-transparent, and fills four vertices. A vertex at radius r and
 *           phase p is the published centre plus the func_801782FC sample of p
 *           scaled by r for x and the func_801783C8 sample of p scaled by r for
 *           y, both shifted right twelve; the radius and phase are written into
 *           the cells 0x1F800006 and 0x1F800008 first and read back inside the
 *           products. Every phase is the work object's word 0x14 plus a fixed
 *           turn and masked to twelve bits.
 *           The first quad uses radius 0x40 at phase word plus 0x200, radius
 *           0x38 at plus 0x230, the bare centre as its third vertex, and the
 *           tip radius 8 at plus 0x400. Its third vertex phase is preceded by a
 *           store of the constant 0x400 into the phase cell, which the later
 *           masked store of the same vertex overwrites before any read.
 *           The second quad uses radius 0x40 at plus 0x200, radius 0x38 at plus
 *           0x1D0, the bare centre, and the tip radius 8 at the unshifted phase
 *           word, so its two side edges mirror the first quad's about the half
 *           turn.
 *           Both quads carry the colour (0x10, 0x10, 0x60) on the two radius
 *           vertices and (0x10, 0x10, 0x30) on the centre and tip vertices, and
 *           both are linked as the 0x24-byte gouraud-quad size under slot 3.
 *           The primitive is reached as a byte pointer, which is the access
 *           form the original shows: every field store is a `sh` or `sb` at a
 *           fixed displacement of the cursor, and the byte form is what keeps
 *           the cell loads of one vertex after that vertex's store instead of
 *           hoisting them over it.
 *           The 0x38-byte frame keeps $ra, $s6 to $s0 and the fifth SetDrawMode
 *           argument at 0x10. The roles of the work object's word 0x14 and
 *           halfwords 0x2E and 0x30 beyond this published geometry are not
 *           established, so they stay offset-anchored, and the scratchpad cells
 *           are named by address for the same reason.
 * @status partial
 * @match 94.70
 * @residual Live measurement 304 of 321 instructions against the original 318;
 *           the body is one instruction longer than the original and differs
 *           only in scheduling. The redundant 0x400 phase store is issued as
 *           `lui` plus `ori` plus `sh` where the original forms it as `lui $at`
 *           plus `sh`, and the remaining differences are the placement of the
 *           hoisted 0x40, 0x60 and 0x30 constants, of the work-pointer loads at
 *           the second and third vertices, and two load-delay `nop`s. No
 *           spelling tried reaches the original's $at form for that store
 *           without changing the rest of the body.
 */
void drawTwoRotatedTranslucentGouraudQuadsAroundWorkHalfwords2EAnd30WithFixedRadii40And38AndTip8GradedDarkBlueBmagicMagic00603_801EFB6C(
    void) {
  u8 *primitive;
  s16 *tip;
  u8 *work;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  tip = &D_1F800004;
  *tip = 8;
  work = g_battle_work;
  D_1F800000 = *(u16 *)(work + 0x2E);
  D_1F800002 = *(u16 *)(work + 0x30);

  primitive = g_PrimCursor;
  SetPolyG4((POLY_G4 *)primitive);
  SetSemiTrans((POLY_G4 *)primitive, 1);

  D_1F800006 = 0x40;
  D_1F800008 = (*(u32 *)(g_battle_work + 0x14) + 0x200) & 0xFFF;
  *(u16 *)(primitive + 0x08) =
      D_1F800000 + ((func_801782FC(D_1F800008) * D_1F800006) >> 12);
  *(u16 *)(primitive + 0x0A) =
      D_1F800002 + ((func_801783C8(D_1F800008) * D_1F800006) >> 12);

  D_1F800006 = 0x38;
  D_1F800008 = (*(u32 *)(g_battle_work + 0x14) + 0x230) & 0xFFF;
  *(u16 *)(primitive + 0x10) =
      D_1F800000 + ((func_801782FC(D_1F800008) * D_1F800006) >> 12);
  *(u16 *)(primitive + 0x12) =
      D_1F800002 + ((func_801783C8(D_1F800008) * D_1F800006) >> 12);

  *(u16 *)(primitive + 0x18) = D_1F800000;
  *(u16 *)(primitive + 0x1A) = D_1F800002;

  SPAD_REF(volatile u16, 0x08) = 0x400;
  D_1F800008 = (*(u32 *)(g_battle_work + 0x14) + 0x400) & 0xFFF;
  *(u16 *)(primitive + 0x20) =
      D_1F800000 + ((func_801782FC(D_1F800008) * *tip) >> 12);
  *(u16 *)(primitive + 0x22) =
      D_1F800002 + ((func_801783C8(D_1F800008) * *tip) >> 12);

  primitive[0x04] = 0x10;
  primitive[0x05] = 0x10;
  primitive[0x06] = 0x60;
  primitive[0x0C] = 0x10;
  primitive[0x0D] = 0x10;
  primitive[0x0E] = 0x60;
  primitive[0x14] = 0x10;
  primitive[0x15] = 0x10;
  primitive[0x16] = 0x30;
  primitive[0x1C] = 0x10;
  primitive[0x1D] = 0x10;
  primitive[0x1E] = 0x30;

  GpuAppendPrim(3, 0x24);

  primitive = g_PrimCursor;
  SetPolyG4((POLY_G4 *)primitive);
  SetSemiTrans((POLY_G4 *)primitive, 1);

  D_1F800006 = 0x40;
  D_1F800008 = (*(u32 *)(g_battle_work + 0x14) + 0x200) & 0xFFF;
  *(u16 *)(primitive + 0x08) =
      D_1F800000 + ((func_801782FC(D_1F800008) * D_1F800006) >> 12);
  *(u16 *)(primitive + 0x0A) =
      D_1F800002 + ((func_801783C8(D_1F800008) * D_1F800006) >> 12);

  D_1F800006 = 0x38;
  D_1F800008 = (*(u32 *)(g_battle_work + 0x14) + 0x1D0) & 0xFFF;
  *(u16 *)(primitive + 0x10) =
      D_1F800000 + ((func_801782FC(D_1F800008) * D_1F800006) >> 12);
  *(u16 *)(primitive + 0x12) =
      D_1F800002 + ((func_801783C8(D_1F800008) * D_1F800006) >> 12);

  *(u16 *)(primitive + 0x18) = D_1F800000;
  *(u16 *)(primitive + 0x1A) = D_1F800002;

  D_1F800008 = *(u32 *)(g_battle_work + 0x14) & 0xFFF;
  *(u16 *)(primitive + 0x20) =
      D_1F800000 + ((func_801782FC(D_1F800008) * *tip) >> 12);
  *(u16 *)(primitive + 0x22) =
      D_1F800002 + ((func_801783C8(D_1F800008) * *tip) >> 12);

  primitive[0x04] = 0x10;
  primitive[0x05] = 0x10;
  primitive[0x06] = 0x60;
  primitive[0x0C] = 0x10;
  primitive[0x0D] = 0x10;
  primitive[0x0E] = 0x60;
  primitive[0x14] = 0x10;
  primitive[0x15] = 0x10;
  primitive[0x16] = 0x30;
  primitive[0x1C] = 0x10;
  primitive[0x1D] = 0x10;
  primitive[0x1E] = 0x30;

  GpuAppendPrim(3, 0x24);
}
