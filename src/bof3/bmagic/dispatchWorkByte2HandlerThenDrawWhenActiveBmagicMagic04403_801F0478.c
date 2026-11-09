#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materializes this cell
 * with its own lui/lw pair for the dispatch index and again after the indirect
 * call, so it is read through this named cell instead of the constant-address
 * accessor, whose folded address stays live across the call in a saved
 * register and grows the frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Three-entry overlay step-handler table for work-object byte 2, based at
 * 0x801F0A2C in this overlay's trailing read-only block: entry 0 = 0x801F04F0,
 * entry 1 = 0x801F05AC, entry 2 = 0x801F0638, the three state blocks that
 * follow this dispatcher. The original adds the scaled byte to this word, so
 * the extern is declared at the first entry, not at the start of the run
 * (0x801F0A10) that holds the byte 1 table at 0x801F0A14. */
extern void (*D_801F0A2C[])(void); /* @source 0x801F0A2C @kind data */

/* Overlay quad emitter this dispatcher invokes once the work object is active:
 * it calls GetGraphType, SetDrawMode, func_80155A08, SetPolyFT4 and
 * SetSemiTrans and draws a translucent textured quad. It was still unlifted
 * when this body was seeded and kept its raw name then; it has since been
 * lifted to exact as
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8
 * at the same address, so only this declaration changed here. */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8(
    void);

/* @source 0x801F0478
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 into a
 * work pointer and uses the work object's byte at offset 0x2, scaled by four,
 * to select one entry of the three-entry handler table at 0x801F0A2C
 * (0x801F04F0, 0x801F05AC, 0x801F0638), which it calls with no arguments. The
 * original does not check the index. When the handler returns, the pointer cell
 * is read again in a second load; if the work object's byte at offset 0x0 and
 * its byte at offset 0x2 are both non-zero it calls this overlay's quad emitter
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8.
 * The two pointer reads are separate locals because the
 * original materializes the cell twice, before and after the indirect call, and
 * the intervening call is what invalidates the first value. The two guards are
 * one conjunction: both tests branch to the same epilogue label. Reads no other
 * state, stores nothing but the saved return address and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic04403_801F0478(void) {
  u8 *stepWork;
  u8 *gateWork;

  stepWork = g_battle_work;
  D_801F0A2C[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5fBmagicMagic04403_801F06D8();
  }
}
