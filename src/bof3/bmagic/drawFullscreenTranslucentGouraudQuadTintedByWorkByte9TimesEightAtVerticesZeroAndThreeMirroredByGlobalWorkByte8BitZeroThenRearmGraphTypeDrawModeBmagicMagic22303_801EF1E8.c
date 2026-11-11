#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell this overlay steps, spelled through the
 * same address-derived symbol the target's other exact lifts use. The body
 * reads the cell once and takes unsigned byte 9 through it twice as the tint
 * source. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record of the main executable at 0x801EB4E0. Only byte 8
 * is read here, as the mirror selector; the generated undefined_syms_auto.txt
 * row already binds the address, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad word cell at 0x1F800008. The body takes this cell's address once,
 * which is why the original keeps the full address in a register (lui /
 * addiu) and then uses offset 0 for the word store and for each of the three
 * byte re-reads instead of folding the low half into the displacement, so the
 * spelling below works through a byte pointer. The generated
 * undefined_syms_auto.txt row already binds the address. */
extern u32 D_1F800008; /* @source 0x1F800008 @kind data */

/* Scratchpad byte cell at 0x1F80000C, the second tint word the body publishes.
 * The two byte re-reads take this cell through its own address symbol while the
 * word store keeps the plain constant address 0x1F80000C, so the store expands
 * as lui $at / sw %lo($at) and each re-read as lui / lbu %lo; spelling both
 * through the constant folds them into one materialized address register and
 * costs three instructions. A D_<ADDRESS> declaration binds on its own. */
extern u8 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Shared main-executable graph-type query bound by this target's generated
 * support file; it takes no arguments and returns the integer graph type. */
s32 GetGraphType(void);

/* @source 0x801EF1E8
 * @behavior Full-screen translucent quad drawer of this overlay, the draw step
 *           the work step at 0x801EF050 reaches through its jal at 0x801EF0A0
 *           while work byte 0 is set. It takes no arguments and returns
 *           nothing. It first picks a draw-mode word by graph type: 0xA5 when
 *           GetGraphType returns 1, 0xA5 again when the second call returns 2,
 *           and 0x35 for every other type; the arms share one store, so the
 *           first comparison hoists its constant into the branch delay slot.
 *           It seeds SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and that
 *           word, and appends the 0xC-byte draw-mode packet with
 *           GpuAppendPrim(2, 0xC).
 *           It then takes a POLY_G4 at g_PrimCursor, seeds it with SetPolyG4,
 *           marks it semi-transparent with the argument 1, and writes the four
 *           vertices of the 320x240 screen rectangle. When bit 0 of byte 8 of
 *           the shared record at 0x801EB4E0 is set the vertices run x0 0,
 *           x1 0x13F, x2 0, x3 0x13F; otherwise x0 0x13F, x1 0, x2 0x13F,
 *           x3 0, so that flag mirrors the quad. y0, y1 are 0 and y2, y3 are
 *           0xEF in both arms, the last one written after the merge.
 *           The tint comes from work byte 9, reached through a single read of
 *           the 0x1F800044 cell. The body publishes that byte as a word
 *           through a pointer to 0x1F800008, publishes eight times that byte
 *           as a word at 0x1F80000C, and then fills the four vertices with
 *           grey, all three channels equal: vertices 0 and 3 take the low byte
 *           of the shifted scratch word, vertices 1 and 2 take the low byte of
 *           the published work byte. Neither the scratch word nor the record
 *           byte 8 has an established role beyond this use, so both stay
 *           address-anchored. The quad is closed with GpuAppendPrim(2, 0x24).
 *           The tail re-arms the draw mode for whatever the caller emits next:
 *           the same graph-type selection with 0x25 for types 1 and 2 and 0x15
 *           for every other type, SetDrawMode at g_PrimCursor with dfe 0 and
 *           dtd 1, and a second 0xC-byte draw-mode packet.
 *           The 0x20-byte frame keeps $ra at 0x1C and $s0 at 0x18 across the
 *           calls. An analysis data-candidates run over this target reported no
 *           range, and this span opens with addiu $sp,-0x20 and closes in jr $ra
 *           through its own frame, so the data lever did not apply. It starts at
 *           its own declared entry and ends where the reviewed 0x801EF3C0
 *           dispatch row starts, so the carve lever did not apply. A byte search
 *           over every payload under out/binaries, both exact and with every
 *           call and branch immediate word masked, found exactly one copy of
 *           this 0x1D8-byte span, the span's own bytes, so the clone lever was
 *           declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFullscreenTranslucentGouraudQuadTintedByWorkByte9TimesEightAtVerticesZeroAndThreeMirroredByGlobalWorkByte8BitZeroThenRearmGraphTypeDrawModeBmagicMagic22303_801EF1E8(
    void) {
  POLY_G4 *prim;
  u8 *work;
  u32 *shade;
  s32 mode;

  if (GetGraphType() == 1) {
    mode = 0xA5;
  } else if (GetGraphType() == 2) {
    mode = 0xA5;
  } else {
    mode = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(2, 0xC);

  prim = (POLY_G4 *)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  if (D_801EB4E0[8] & 1) {
    prim->x0 = 0;
    prim->y0 = 0;
    prim->x1 = 0x13F;
    prim->y1 = 0;
    prim->x2 = 0;
    prim->y2 = 0xEF;
    prim->x3 = 0x13F;
    prim->y3 = 0xEF;
  } else {
    prim->x0 = 0x13F;
    prim->y0 = 0;
    prim->x1 = 0;
    prim->y1 = 0;
    prim->x2 = 0x13F;
    prim->y2 = 0xEF;
    prim->x3 = 0;
    prim->y3 = 0xEF;
  }

  work = g_battle_work;
  shade = &D_1F800008;
  *shade = work[9];
  prim->r0 = prim->r3 = (SPAD_REF(u32, 0xCu) = work[9] << 3);
  prim->g0 = prim->g3 = D_1F80000C;
  prim->b0 = prim->b3 = D_1F80000C;
  prim->r1 = prim->r2 = *(u8 *)shade;
  prim->g1 = prim->g2 = *(u8 *)shade;
  prim->b1 = prim->b2 = *(u8 *)shade;
  GpuAppendPrim(2, 0x24);

  if (GetGraphType() == 1) {
    mode = 0x25;
  } else if (GetGraphType() == 2) {
    mode = 0x25;
  } else {
    mode = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
  GpuAppendPrim(2, 0xC);
}
