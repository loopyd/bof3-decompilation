#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, already bound at that
 * address by this target's map as g_battle_work, the name this overlay's exact
 * lifts add16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic03903_801EF690 and
 * add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvoke
 * HelperWhenZeroBmagicMagic03903_801EF6E0 declare the same way. The original
 * re-materialises the cell with its own lui 0x1F80 plus lw 68($vN) pair at each
 * access group, because a store through the loaded pointer may alias the cell,
 * so the cell is named rather than written as a bare scratchpad constant. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad centre cells at 0x1F800000, 0x1F800002 and 0x1F800004, the three
 * halfwords this body publishes from the 3-byte-stride palette row at
 * 0x801F040C and then re-reads as the red, green and blue channels of the two
 * tinted corners. Every read is its own lui 0x1F80 plus lhu pair, so each cell
 * is one volatile halfword addressed absolutely rather than a value carried in
 * a register. */
extern volatile u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern volatile u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern volatile u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad size cells at 0x1F800006 and 0x1F800008, the height published
 * from work byte 0x09 and the half width published from work byte 0x0A. Each
 * vertex re-reads its cell with a fresh lui 0x1F80 plus lhu pair instead of
 * reusing the previous vertex's value, which is what the volatile qualifier
 * forces; the arithmetic stays unsigned, so no sign-extension sequence appears
 * around either subtraction. */
extern volatile u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern volatile u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Scratchpad centre cell at 0x1F80000A, the x halfword published from work
 * halfword 0x2E. This body forms its address once as one lui 0x1F80 plus
 * addiu pair, keeps that base in $a0 across all four x vertices and reads it at
 * offset zero, so the cell is a live pointer to a halfword rather than a folded
 * absolute constant. */
extern volatile u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Scratchpad centre cell at 0x1F80000C, the y halfword published from work
 * halfword 0x30. It is read absolutely and separately for every vertex, and
 * once more for the last corner after the first six colour bytes, so it keeps
 * one reference per access and no local copy exists to hoist. */
extern volatile u16 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Three byte rows of the palette record at 0x801F040C, indexed by the work
 * object's byte 0x04 scaled by three. The original masks each row with its own
 * %hi/%lo pair over a base of 0x801F040C, 0x801F040D and 0x801F040E, so the
 * three rows are separate symbols and the stride is what selects the record. */
extern u8 D_801F040C[]; /* @source 0x801F040C @kind table */
extern u8 D_801F040D[]; /* @source 0x801F040D @kind table */
extern u8 D_801F040E[]; /* @source 0x801F040E @kind table */

void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801EF758
 * @behavior Per-frame battle-overlay draw handler. It publishes the work
 *           object's byte 0x09 into scratchpad halfword 0x1F800006, byte 0x0A
 *           into 0x1F800008, halfwords 0x2E and 0x30 into 0x1F80000A and
 *           0x1F80000C, and the palette record selected by work byte 0x04 into
 *           0x1F800000, 0x1F800002 and 0x1F800004. It then picks the texture
 *           page from the graph type (0xA5 for types 1 and 2, 0x35 otherwise),
 *           emits a draw mode and links a 0xC-byte packet, and builds one
 *           translucent Gouraud quad whose corners are (cx - halfWidth,
 *           cy - height), (cx + halfWidth, cy - height), (cx - halfWidth, cy)
 *           and (cx + halfWidth, cy), with the two upper corners graded black
 *           and the two lower corners tinted by the published palette row. It
 *           closes by linking the 0x24-byte quad.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadAboveWorkHalfwords2EAnd30WithHalfWidthFromWorkByteAAndHeightFromWorkByte9GradedBlackToPaletteRowAtWorkByte4BmagicMagic03903_801EF758(
    void) {
  volatile POLY_G4 *primitive;
  volatile u16 *centre;
  s32 tpage;
  u16 lastY;
  u8 *work;

  work = g_battle_work;
  SPAD_REF(u16, 0x06) = work[0x09];
  SPAD_REF(u16, 0x08) = work[0x0A];
  SPAD_REF(u16, 0x0A) = *(u16 *)(work + 0x2E);
  SPAD_REF(u16, 0x0C) = *(u16 *)(work + 0x30);
  SPAD_REF(u16, 0x00) = D_801F040C[work[0x04] * 3];
  SPAD_REF(u16, 0x02) = D_801F040D[work[0x04] * 3];
  SPAD_REF(u16, 0x04) = D_801F040E[work[0x04] * 3];

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                0, 0xC);

  primitive = (volatile POLY_G4 *)g_PrimCursor;
  SetPolyG4((POLY_G4 *)primitive);
  SetSemiTrans((POLY_G4 *)primitive, 1);

  centre = &D_1F80000A;
  primitive->x0 = centre[0] - D_1F800008;
  primitive->y0 = D_1F80000C - D_1F800006;
  primitive->x1 = centre[0] + D_1F800008;
  primitive->y1 = D_1F80000C - D_1F800006;
  primitive->x2 = centre[0] - D_1F800008;
  primitive->y2 = D_1F80000C;
  primitive->x3 = centre[0] + D_1F800008;
  lastY = D_1F80000C;

  primitive->r0 = 1;
  primitive->g0 = 1;
  primitive->b0 = 1;
  primitive->r1 = 1;
  primitive->g1 = 1;
  primitive->b1 = 1;
  primitive->y3 = lastY;
  primitive->r2 = D_1F800000;
  primitive->g2 = D_1F800002;
  primitive->b2 = D_1F800004;
  primitive->r3 = D_1F800000;
  primitive->g3 = D_1F800002;
  primitive->b3 = D_1F800004;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                0, 0x24);
}
