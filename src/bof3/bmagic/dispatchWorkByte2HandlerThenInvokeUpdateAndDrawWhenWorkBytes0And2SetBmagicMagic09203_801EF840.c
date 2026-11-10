#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materialises the cell with its own
 * lui/lw pair for the dispatch index and again with a second lui/lw pair after
 * the indirect call, because that call may republish the pointer, so both reads
 * stay on this named cell rather than on a pointer local, as in the exact
 * bmagic siblings
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic00903_801EF120
 * and
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes2And0SetBmagicMagic17203_801EF518,
 * which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 7 of the fifteen-word code-pointer run based at 0x801F07BC, which Splat
 * places as the single textbin row T_801F07BC: 0x801EF8C0, 0x801EF940,
 * 0x801EF990, 0x801EFA24, 0x801F0320, 0x801F038C, 0x801F03DC and 0x801F0420.
 * The work record's byte 2 scales by four and is added to 0x801F07D8, so the
 * extern is declared at the indexed word and the run stays that one map row;
 * the generated undefined_syms_auto.txt row for 0x801F07D8 already names this
 * word. */
extern void (*D_801F07D8[])(void); /* @source 0x801F07D8 @kind data */

/* Shared main-executable update pass at 0x800AF764. The payload calls it once
 * per active step with no arguments and no result; the generated
 * undefined_funcs_auto.txt row for that address binds it. */
void func_800AF764(void);

/* This overlay's own draw pass at 0x801EFB04, lifted exact in
 * src/bof3/bmagic/drawTranslucentTexturedQuadBandOfFourQuadsPerRowSteppingSixTexelsFromRowFourToThirtyOneScaledByWorkByte9AndWorkByteABmagicMagic09203_801EFB04.c:
 * one jal at 0x801EF8A8 is the only word of this payload that holds the
 * address. It selects a draw mode from GetGraphType, installs it with
 * SetDrawMode on the primitive cursor, links packets and emits clut- and
 * tpage-sourced textured quads through SetSemiTrans, GetClut and GetTPage. */
void drawTranslucentTexturedQuadBandOfFourQuadsPerRowSteppingSixTexelsFromRowFourToThirtyOneScaledByWorkByte9AndWorkByteABmagicMagic09203_801EFB04(
    void);

/* @source 0x801EF840
 * @behavior Dispatches one step of the overlay work handler and then updates and
 *   draws the active record. The work record is read from the scratchpad
 *   pointer cell at 0x1F800044, its unsigned byte at offset 0x2 is scaled by
 *   four and selects one entry of the handler table word at 0x801F07D8, which
 *   is called with no arguments; the original does not bound the index. The
 *   cell is then re-read and the record's bytes at offsets 0x0 and 0x2 are
 *   tested in that order, so only when both are non-zero does the arm run: it
 *   calls the shared main-executable update pass func_800AF764 and then this
 *   overlay's own draw pass
 *   drawTranslucentTexturedQuadBandOfFourQuadsPerRowSteppingSixTexelsFromRowFourToThirtyOneScaledByWorkByte9AndWorkByteABmagicMagic09203_801EFB04,
 *   both with no arguments. The cell is
 *   re-read rather than kept live because the dispatched handler may republish
 *   it, which is why the original materialises it twice. The 0x18-byte frame
 *   only keeps $ra at 0x10, and the pointer-cell load is issued before frame
 *   creation. No table word or jal in this payload names the 0x80-byte span, so
 *   it is reached through the overlay step chain; it takes no arguments and
 *   returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenWorkBytes0And2SetBmagicMagic09203_801EF840(
    void) {
  D_801F07D8[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_800AF764();
    drawTranslucentTexturedQuadBandOfFourQuadsPerRowSteppingSixTexelsFromRowFourToThirtyOneScaledByWorkByte9AndWorkByteABmagicMagic09203_801EFB04();
  }
}
