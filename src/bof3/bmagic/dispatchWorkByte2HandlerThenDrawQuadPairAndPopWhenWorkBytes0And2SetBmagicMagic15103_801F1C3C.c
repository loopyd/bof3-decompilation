#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original re-materialises the cell
 * address after the indirect call (lui $v1,0x1F80 / lw $v1,0x44($v1)), so the
 * cell is read through the named target symbol instead of the constant-address
 * accessor, as in the exact bmagic twin
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Three-entry handler table for work-record byte 2. It starts at 0x801F250C,
 * 0x68 bytes into the pointer run whose row begins at 0x801F24A4, and holds the
 * three work byte 2 handlers that follow this dispatcher: 0x801F1CCC,
 * 0x801F1D28 and 0x801F1D60. The sub-table address is bound by this target's
 * symbols.txt because the run's own row starts 0x68 bytes earlier. */
extern void (*workByte2HandlerTable[])(void); /* @source 0x801F250C @kind data */

/* Installs the work record's transform: pushes the GTE matrix stack, builds the
 * record's translation and rotation and sets both GTE matrices. */
void projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic15103_801F1DC0(
    void);

/* Draws the first quad ring of the active work record: sixty-four
 * semi-transparent Gouraud quads between radius work word 0x14 and that radius
 * plus 0x40 through SetPolyG4, SetSemiTrans and RotTransPers4, tinted by work
 * byte 0x09 times fifteen on the outer edge. It is lifted beside this caller,
 * so the declaration carries its evidenced name. */
void drawTranslucentGouraudQuadRingAtWorkWord14RadiusSixtyFourWideInSixtyFourStepsTintingOuterEdgeByWorkByte9TimesFifteenBmagicMagic15103_801F1E70(
    void);

/* Draws the second quad of the active work record through the same calls with a
 * different vertex-colour seed. */
void func_801F2188(void);

/* @source 0x801F1C3C
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work record published at the pointer cell 0x1F800044:
 *           the record byte at offset 0x2, scaled by four, selects one entry of
 *           the three-entry handler table at 0x801F250C (0x801F1CCC,
 *           0x801F1D28, 0x801F1D60) and that entry is invoked with no
 *           arguments. After the handler returns the pointer cell is read a
 *           second time; only when the record byte at offset 0x0 and the byte
 *           at offset 0x2 are both non-zero does it call
 *           projectWorkObjectPositionAndInstallTransformWithZeroTranslation
 *           BmagicMagic15103_801F1DC0, which pushes the GTE matrix stack and
 *           installs the record's transform,
 *           then the two quad drawers
 *           drawTranslucentGouraudQuadRingAtWorkWord14RadiusSixtyFourWide
 *           InSixtyFourStepsTintingOuterEdgeByWorkByte9TimesFifteen
 *           BmagicMagic15103_801F1E70 and func_801F2188, then
 *           PopMatrix, restoring the stack it pushed. Reads no other state,
 *           stores nothing directly and returns nothing; the 0x18-byte frame
 *           only keeps $ra across the calls. The range is one 0x90-byte
 *           function: the map rows that split it at 0x801F1C7C (a consumer-less
 *           textbin row over live code) and at 0x801F1CBC (this frame's own
 *           epilogue) were merged into this row.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawQuadPairAndPopWhenWorkBytes0And2SetBmagicMagic15103_801F1C3C(
    void) {
  u8 *step_record;
  u8 *draw_record;

  step_record = g_battle_work;
  workByte2HandlerTable[step_record[2]]();
  draw_record = g_battle_work;
  if (draw_record[0] != 0 && draw_record[2] != 0) {
    projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic15103_801F1DC0();
    drawTranslucentGouraudQuadRingAtWorkWord14RadiusSixtyFourWideInSixtyFourStepsTintingOuterEdgeByWorkByte9TimesFifteenBmagicMagic15103_801F1E70();
    func_801F2188();
    PopMatrix();
  }
}
