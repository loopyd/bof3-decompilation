#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay's exact lifts
 * add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenZeroBmagicMagic00603_801F0290 and
 * dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenActiveBmagicMagic00603_
 * 801F0064 already use. This body reads the cell three times: once into a local
 * for the four publishes that precede the draw-mode call, and once for each
 * func_80155A08 call site, where the two arguments of one call are read through a
 * single cell load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad centre cell at 0x1F800000, the x halfword this body publishes and
 * then re-reads as the base of every one of the four x vertices. The original
 * forms its address as one lui 0x1F80 plus addiu pair and keeps that base in $a0
 * across all four reads, so the cell is a live pointer to a halfword, not a
 * folded constant. The family sibling
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWork
 * ByteATimesBytes5d5e5fBmagicMagic04403_801F06D8 reads the same cell the same
 * way, and the still-asm span func_801F0790 of this target also binds it with a
 * lui %hi / addiu %lo pair, so it keeps its splat-generated name. */
extern volatile u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad halfword cells at 0x1F800002, 0x1F800004 and 0x1F800006, the
 * remaining three cells this body publishes and then reads back inside the
 * vertex expressions. Every read re-forms its address with its own lui 0x1F80
 * and its own load, and no read is reused, so each cell is one volatile halfword
 * object addressed absolutely rather than a displacement of the 0x1F800000 base.
 * The volatile qualifier is what reproduces that: unqualified cells let the
 * compiler carry all four corner loads in registers and reuse them across the
 * primitive stores, which measured 84 of 123 instructions at 408 bytes. The
 * three addresses are bound by this target's symbols.txt because splat names
 * only the 0x1F800000 cell from the original assembly. */
extern volatile u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern volatile u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern volatile u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Shared primitive linker at 0x80155A08: resolves the (x, y) screen cell, links
 * the primitive at g_PrimCursor with the given kind and advances the cursor by
 * the given size. This body links a 0xC-byte packet at the work object words
 * 0x34 and 0x38 before the quad and the 0x24-byte quad itself after it, both
 * with kind 0. Declared locally because the header chain has no row for it and
 * this target's generated undefined_funcs_auto.txt already binds the address. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

s32 GetGraphType(void);

/* @source 0x801F0308
 * @behavior Draw routine of this overlay's work-byte-2 handler group. Its only
 *           in-image reference is the jal at 0x801F00E8 inside
 *           dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenActiveBmagic
 *           Magic00603_801F0064, taken after the shared helper at 0x800AF764
 *           while the work object's byte 0 and its byte 2 are both non-zero. The
 *           body reads no incoming register, so it takes no arguments and
 *           returns nothing.
 *           It reads the scratchpad work-object pointer cell once and publishes
 *           four halfwords into the scratchpad before anything else: the work
 *           object's unsigned byte 0x9 into the cell at 0x1F800004, its unsigned
 *           byte 0xA into 0x1F800006, and its halfwords 0x2E and 0x30 into
 *           0x1F800000 and 0x1F800002. Each of those four stores re-forms its
 *           target with its own lui 0x1F80, so all four cells keep the absolute
 *           address form.
 *           It then selects the draw-mode texture page 0xA5 while the shared
 *           graphic type is 1 or 2 and 0x35 otherwise, calling GetGraphType
 *           once per test with no cached result, and links a 0xC-byte draw-mode
 *           packet through SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and a
 *           null RECT. Next func_80155A08 links a 0xC-byte packet at the work
 *           object's words 0x34 and 0x38 with kind 0.
 *           The primitive cursor is then loaded once and kept in $s0 across the
 *           quad: SetPolyG4 sets the gouraud-quad tag and SetSemiTrans(prim, 1)
 *           marks it semi-transparent. The four vertices place a band whose far
 *           edge is y minus the byte 0x9 cell and whose near edge is y, running
 *           from x minus the byte 0xA cell to x plus it, where x and y are the
 *           published halfwords 0x2E and 0x30: x0 = x - bA, y0 = y - b9,
 *           x1 = x + bA, y1 = y - b9, x2 = x - bA, y2 = y, x3 = x + bA,
 *           y3 = y. The x cell is re-read before every vertex through the one
 *           centre pointer and the other three cells keep one lui 0x1F80 plus
 *           one load each, because the halfword stores through the primitive may
 *           alias them.
 *           The primitive is reached through a volatile pointer. That is the
 *           qualifier the original's schedule requires: with every corner store
 *           a volatile access, each corner's two cell loads stay in its own
 *           statement and the load delay slots of the second load stay empty,
 *           while the four published colour constants still fill the slots
 *           around them. Without it the compiler carries the corner loads across
 *           the stores and the body measures 80 of 123 instructions.
 *           The far edge carries colour (1, 1, 1) and the near edge
 *           (0x80, 0x20, 1), a black-to-dark-blue gradient down the band. The
 *           original reads the y3 cell right after the x3 store but writes y3
 *           only between the g3 and b3 colour stores, so that one corner is read
 *           into a local and stored with the colours; the local is a halfword so
 *           the store needs no truncation mask.
 *           Finally func_80155A08 links the 0x24-byte quad itself at the same
 *           two work words through a second cell load, and the body returns.
 *           The 0x20-byte frame only keeps $ra at 0x1C and $s0 at 0x18, and the
 *           pointer load of the first publish group precedes frame creation as
 *           the original schedules it.
 *           The roles of work bytes 0x9 and 0xA and of work halfwords 0x2E and
 *           0x30 beyond this published geometry are not established, so they
 *           stay offset-anchored, and the four scratchpad cells are named by
 *           address rather than by role for the same reason.
 *           The 492-byte span was compared against every extracted payload under
 *           out/binaries with its 28 jal, j and lui words masked and has no
 *           twin, so the clone lever offered no source and the body was written
 *           from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadAroundWorkHalfwords2EAnd30WithHalfExtentsFromWorkBytes9AndAGradedBlackToDarkBlueBmagicMagic00603_801F0308(
    void) {
  volatile POLY_G4 *primitive;
  volatile u16 *centre;
  s32 graph;
  u16 lastY;
  u8 *work;

  work = g_battle_work;
  SPAD_REF(u16, 0x04) = work[0x09];
  SPAD_REF(u16, 0x06) = work[0x0A];
  SPAD_REF(u16, 0x00) = *(u16 *)(work + 0x2E);
  SPAD_REF(u16, 0x02) = *(u16 *)(work + 0x30);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                0, 0xC);

  primitive = (volatile POLY_G4 *)g_PrimCursor;
  SetPolyG4((POLY_G4 *)primitive);
  SetSemiTrans((POLY_G4 *)primitive, 1);

  centre = &D_1F800000;
  primitive->x0 = centre[0] - D_1F800006;
  primitive->y0 = D_1F800002 - D_1F800004;
  primitive->x1 = centre[0] + D_1F800006;
  primitive->y1 = D_1F800002 - D_1F800004;
  primitive->x2 = centre[0] - D_1F800006;
  primitive->y2 = D_1F800002;
  primitive->x3 = centre[0] + D_1F800006;
  lastY = D_1F800002;

  primitive->r0 = 1;
  primitive->g0 = 1;
  primitive->b0 = 1;
  primitive->r1 = 1;
  primitive->g1 = 1;
  primitive->b1 = 1;
  primitive->r2 = 0x80;
  primitive->g2 = 0x20;
  primitive->b2 = 1;
  primitive->r3 = 0x80;
  primitive->g3 = 0x20;
  primitive->y3 = lastY;
  primitive->b3 = 1;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38),
                0, 0x24);
}
