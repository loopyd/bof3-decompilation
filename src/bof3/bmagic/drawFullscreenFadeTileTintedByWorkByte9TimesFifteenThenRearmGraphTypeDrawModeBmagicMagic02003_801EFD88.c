#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A named
 * cell symbol folds the constant address into the load (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)); the body reads the cell once and takes unsigned byte 9
 * through it as the tint source. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Scratchpad halfword cell at 0x1F800002. The body takes this cell's address
 * once, which is why the original keeps the full address in a register (lui /
 * addiu) and then uses offset 0 for the store and both re-reads instead of
 * folding the low half into the displacement, so the spelling below works
 * through a halfword pointer. The generated undefined_syms_auto.txt already
 * binds the address, so no target-map row is needed. The cell holds 15 times
 * work-object byte 9 between that store and the two reads that follow the
 * tile's own stores. */
extern u16 D_1F800002; /* @source 0x1F800002 @kind unknown */

/* Shared main-executable graph-type query bound by this target's generated
 * support file; it takes no arguments and returns the integer graph type. */
s32 GetGraphType(void);

/* @source 0x801EFD88
 * @behavior Full-screen fade-tile drawer of this overlay. It is called with no
 *           arguments and returns nothing, from the work step at 0x801EF234 and
 *           from the draw step at 0x801EF0F8. It first picks the draw-mode word
 *           by graph type: 0xA5 when GetGraphType returns 1, 0xA5 again when the
 *           second call returns 2, and 0x35 for every other type; the arm order
 *           in the original hoists the taken arm's constant into the branch
 *           delay slot, so both comparisons branch to one shared store. It seeds
 *           SetDrawMode at g_PrimCursor with dfe 0 and dtd 1 and appends the
 *           0xC-byte draw-mode packet with GpuAppendPrim(2, 0xC).
 *           It then takes a TILE at g_PrimCursor, seeds it with SetTile, marks
 *           it semi-transparent with the argument 1, and writes the full-screen
 *           rectangle w 0x140, y0 0, x0 0, h 0xF0 in that store order. The tint
 *           comes from work byte 9: the body loads the 0x1F800044 cell once,
 *           multiplies that unsigned byte by 15 with the shift-and-subtract the
 *           original uses, stores the product through a pointer to the
 *           scratchpad halfword at 0x1F800002 and reads that halfword back for
 *           each of r0, g0 and b0, so the three colour bytes take 15 times work
 *           byte 9 truncating to a byte and the stores through the tile pointer
 *           re-read the cell rather than reusing the stored register. The tile
 *           is closed with GpuAppendPrim(2, 0x10).
 *           The tail re-arms the draw mode for whatever the caller emits next:
 *           the same graph-type selection with 0x25 for types 1 and 2 and 0x15
 *           for every other type, SetDrawMode at g_PrimCursor with dfe 0 and dtd
 *           1, and a second 0xC-byte draw-mode packet.
 *           The 0x20-byte frame keeps $ra at 0x1C and $s0 at 0x18 across the
 *           calls, and nothing is returned. The roles of the two graph-type
 *           draw-mode words and of work byte 9 are not established beyond their
 *           use here, so the words stay literals and the byte stays
 *           offset-anchored.
 *           Levers: an analysis data-candidates run over this target reported no
 *           range, and this span opens with addiu $sp,-0x20 and closes in jr $ra
 *           through its own frame, so the data lever did not apply. It starts at
 *           its own declared entry and ends where the reviewed 0x801EFEB8
 *           textbin row starts, so the carve lever did not apply. A byte search
 *           over every payload under out/binaries, both exact and with every
 *           call and branch immediate word masked in the payload's own
 *           little-endian order, found exactly one copy of this 0x130-byte span,
 *           the span's own bytes, so the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFullscreenFadeTileTintedByWorkByte9TimesFifteenThenRearmGraphTypeDrawModeBmagicMagic02003_801EFD88(
    void) {
  TILE *tile;
  u8 *work;
  u16 *shade;
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

  tile = (TILE *)g_PrimCursor;
  SetTile(tile);
  SetSemiTrans(tile, 1);
  tile->w = 0x140;
  tile->y0 = 0;
  tile->x0 = 0;
  tile->h = 0xF0;
  work = D_1F800044;
  shade = &D_1F800002;
  *shade = work[9] * 15;
  tile->r0 = *shade;
  tile->g0 = *shade;
  tile->b0 = *shade;
  GpuAppendPrim(2, 0x10);

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
