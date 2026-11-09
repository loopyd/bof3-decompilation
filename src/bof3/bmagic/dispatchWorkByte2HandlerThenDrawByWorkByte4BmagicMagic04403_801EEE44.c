#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * with its own lui/lw pair before the frame is created and again after the
 * indirect call, so it is read through this named cell instead of the
 * constant-address accessor, whose folded address stays live across the call in
 * a saved register and grows the frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Three-entry overlay handler table for work-object byte 2, based at
 * 0x801F0A1C in this overlay's trailing read-only pointer run: entry 0 =
 * 0x801EEEFC, entry 1 = 0x801EF138, entry 2 = 0x801EF320, the state blocks that
 * follow this dispatcher. The original adds the scaled byte to this word, so
 * the extern is declared at the first entry, not at the start of the run
 * (0x801F0A10) that holds the byte 1 table at 0x801F0A14. This table sits
 * inside that six-word run, so the run keeps its own boundary and this extern
 * binds the interior word. */
extern void (*D_801F0A1C[])(void); /* @source 0x801F0A1C @kind data */

/* Overlay drawing routines this dispatcher picks between once the work object
 * is active: func_801EF4E0 while work byte 4 is non-zero and
 * drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarter
 * TurnRadiusTwelveBmagicMagic04403_801EFA4C while it is zero. Both call
 * GetGraphType, SetDrawMode and g_PrimCursor primitives, so both bodies draw.
 * func_801EF4E0 is unlifted and keeps its raw name. */
void func_801EF4E0(void);
void drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarterTurnRadiusTwelveBmagicMagic04403_801EFA4C(
    void);

/* @source 0x801EEE44
 * @behavior Entry 0 of the six-word handler run at 0x801F0A14 that this
 * overlay's byte 1 dispatcher dispatchStateByte1Table0a14BmagicMagic04403_801EEE00
 * indexes, so it is entered with no arguments and no jal in this payload
 * targets it. It reads the scratchpad work-object pointer cell at 0x1F800044
 * before it creates its frame, takes the work object's unsigned byte 2, scales
 * it by four and calls that entry of the three-entry handler table at
 * 0x801F0A1C with no arguments; the original does not check the index. When
 * that handler returns the cell is read again in a second load, because the
 * dispatched handler may republish it, and the work object's byte 4 selects the
 * drawing path: a non-zero byte 4 calls func_801EF4E0, a zero byte 4 calls
 * drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarter
 * TurnRadiusTwelveBmagicMagic04403_801EFA4C. Each path runs only while the work
 * object's byte 0 and byte 2 are both non-zero, and the two gate tests branch to
 * the shared epilogue, so
 * each path re-tests the pair rather than sharing one conjunction. Reads no
 * other state, stores nothing but the saved return address and returns
 * nothing. The 0x18-byte frame keeps only $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic04403_801EEE44(void) {
  u8 *stepWork;
  u8 *drawWork;

  stepWork = g_battle_work;
  D_801F0A1C[stepWork[2]]();
  drawWork = g_battle_work;
  if (drawWork[4] != 0) {
    if (drawWork[0] != 0 && drawWork[2] != 0) {
      func_801EF4E0();
    }
  } else if (drawWork[0] != 0 && drawWork[2] != 0) {
    drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarterTurnRadiusTwelveBmagicMagic04403_801EFA4C();
  }
}
