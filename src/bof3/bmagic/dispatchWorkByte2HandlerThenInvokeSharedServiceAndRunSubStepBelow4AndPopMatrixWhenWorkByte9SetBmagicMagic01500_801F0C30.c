#include "bof3/bof3.h"

typedef void (*BmagicMagic01500SevenByte2Handler)(void);

typedef struct BmagicMagic01500SevenByte2DispatchTable {
  BmagicMagic01500SevenByte2Handler handlers[7];
} BmagicMagic01500SevenByte2DispatchTable;

/*
 * Seven-word code pointer run at 0x801EEC84, the table this dispatcher copies:
 * entry 0 = 0x801F0D48, entry 1 = 0x801F0DB4, entry 2 = 0x801F0E54, entry 3 =
 * 0x801F0F2C, entry 4 = 0x801F1088, entry 5 = 0x801F1180 and entry 6 =
 * 0x801F1230, all function starts of this overlay. The run is the interior
 * window 0x20..0x26 of the work byte 2 handler run based at 0x801EEC34 that
 * dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic01500_801EFA28
 * copies in full, so the extern is declared at the word the scaled index adds
 * to and the run stays one row. That word sits in the overlay's leading data
 * block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 */
extern BmagicMagic01500SevenByte2DispatchTable D_801EEC84; /* @source 0x801EEC84 @kind data */

/* Scratchpad work-object pointer cell, spelled as the named cell so the
 * assembler folds the constant address into each load (lui $vN,0x1F80 /
 * lw $vN,0x44($vN)). The original re-reads the cell after the indirect call and
 * again after the shared-service call because the called handlers may rewrite
 * it, and a spelled-out constant address folds to one value the compiler keeps
 * in a callee-saved register across those calls, so the cell is named here
 * instead of read through the scratchpad macro. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* Sub-step emitter pair selected by the work object's byte 2 while it is below
 * three, the projected draw step that follows them, and the GTE matrix stack
 * pop: 0x801F1250 and 0x801F131C carry their reviewed spellings, and the draw
 * step at 0x801F13D4 is the projected eight-triangle fan of the same overlay. */
extern void projectWorkRecordPositionAndInstallTransformWithHalfNegatedRecordPositionDistanceBmagicMagic01500_801F1250(void);
extern void projectWorkRecordPositionAndInstallTransformWithRecordHalfword3EAt8014638CBmagicMagic01500_801F131C(void);
extern void drawProjectedTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusFourTimesWorkByte9BmagicMagic01500_801F13D4(void);

/* @source 0x801F0C30
 * @behavior Work byte 1 handler, entry 3 of the four-entry handler run at
 *           0x801F1630 (0x801EFA28, 0x801EFF1C, 0x801F0334, 0x801F0C30) that
 *           dispatchWorkByte1HandlerBmagicMagic01500_801EF9E4 selects on
 *           unsigned work byte 1. It copies the seven-word work byte 2 handler
 *           run at 0x801EEC84 (0x801F0D48, 0x801F0DB4, 0x801F0E54, 0x801F0F2C,
 *           0x801F1088, 0x801F1180, 0x801F1230) into its own 0x38-byte frame as
 *           inline word copies, then reads the scratchpad work-object pointer
 *           cell at 0x1F800044 and invokes the copied entry selected by that
 *           object's unsigned byte 2 with no arguments and no bounds test. After
 *           the handler returns it re-reads the cell and, when the work object's
 *           byte 0 and byte 2 are both non-zero, invokes the shared
 *           main-executable service func_8014D290; it then re-reads the cell
 *           again and, when that object's byte 9 is non-zero and its unsigned
 *           byte 2 is below 4, runs the emitter selected by byte 2 below three
 *           (0x801F1250) or 0x801F131C, then the projected draw step 0x801F13D4
 *           and the GTE matrix stack pop. It takes no arguments and returns
 *           nothing; the frame keeps the table copy at 0x10 and $ra at 0x30, and
 *           the first cell load is hoisted above the frame adjustment. It reads
 *           and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceAndRunSubStepBelow4AndPopMatrixWhenWorkByte9SetBmagicMagic01500_801F0C30(
    void) {
  BmagicMagic01500SevenByte2DispatchTable handlers;
  u8 *work;
  u8 step;

  handlers = D_801EEC84;
  handlers.handlers[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
    work = D_1F800044;
    if (work[9] != 0) {
      step = work[2];
      if (step < 4) {
        if (step < 3) {
          projectWorkRecordPositionAndInstallTransformWithHalfNegatedRecordPositionDistanceBmagicMagic01500_801F1250();
        } else {
          projectWorkRecordPositionAndInstallTransformWithRecordHalfword3EAt8014638CBmagicMagic01500_801F131C();
        }
        drawProjectedTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusFourTimesWorkByte9BmagicMagic01500_801F13D4();
        PopMatrix();
      }
    }
  }
}
