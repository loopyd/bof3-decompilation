#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell with its own lui/lw pair for the dispatch
 * index and again after the indirect call, so it is read through this named cell
 * instead of the constant-address accessor, whose folded address stays live
 * across the call in a saved register and grows the frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Overlay step-handler table for work-object byte 2, based at 0x801F0BCC inside
 * this overlay's trailing pointer run (the run is declared whole at 0x801F0B64,
 * whose generated row already names the interior words). Its neighbours fix its
 * extent: the byte-2 table at 0x801F0BBC ends one word before this base, and the
 * three-entry run at 0x801F0BDC starts one word after its last entry. The
 * original adds the scaled byte to this word, so the extern is declared at the
 * first entry, not at the start of the run. */
extern void (*D_801F0BCC[])(void); /* @source 0x801F0BCC @kind data */

/* Overlay drawing routine this dispatcher invokes once the work object is
 * active: it calls GetGraphType, then SetDrawMode and func_8014E5A0, builds a
 * polygon with SetPolyG4 and SetSemiTrans, and walks four paint passes through
 * func_801782FC and func_801783C8. Its own body is lifted at that name and
 * measured 224 of the original 240 instructions at the lane that seeded it. */
extern void drawTranslucentRotatedQuadAtScreenCenterScaledAndTintedByWorkByte9BmagicMagic06000_801F0428(void);

/* @source 0x801F0254
 * @behavior Index 1 of the three-entry handler table at 0x801F0BB0 that the
 * work-byte-1 dispatcher at 0x801EFF44 indexes with the work object's byte at
 * offset 0x1, and itself a dispatcher: it reads the scratchpad work-object
 * pointer cell at 0x1F800044 into a work pointer and uses the work object's byte
 * at offset 0x2, scaled by four, to select one entry of the handler table at
 * 0x801F0BCC, which it calls with no arguments. The original does not check the
 * index. When the handler returns the pointer cell is read again in a second
 * load; if the work object's byte at offset 0x0 and its byte at offset 0x2 are
 * both non-zero it calls this overlay's state drawing routine
 * drawTranslucentRotatedQuadAtScreenCenterScaledAndTintedByWorkByte9BmagicMagic06000_801F0428. The
 * two pointer reads are separate locals because the original materializes the
 * cell twice, before and after the indirect call, and the intervening call is
 * what invalidates the first value. The two guards are one conjunction: both
 * tests branch to the same epilogue label. Reads no other state, stores nothing
 * but the saved return address in an 0x18 frame and returns nothing. Seeded from
 * the neighbour at 0x801EFF88, which is index 0 of the same 0x801F0BB0 table and
 * byte-identical to this span except for this table base's low half and this
 * jal target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Table0bccHandlerThenDrawWhenActiveBmagicMagic06000_801F0254(void) {
  u8 *stepWork;
  u8 *gateWork;

  stepWork = g_battle_work;
  D_801F0BCC[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    drawTranslucentRotatedQuadAtScreenCenterScaledAndTintedByWorkByte9BmagicMagic06000_801F0428();
  }
}
