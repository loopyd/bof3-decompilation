#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler keep that base in a saved register across
 * the call and load through offset zero instead, which costs the frame slot.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Draw-move emitter in this overlay at 0x801F41F0. Its own bytes take a grid row
 * index and an 8-pixel column index, build the 0x8 by 0x78 rect
 * x = 0x240 + row * 0x40 + (column + 1) * 8, y = 0x160 at +0x10 of its frame,
 * hand that rect to SetDrawMove with the primitive published in g_PrimCursor and
 * then call func_8014E5A0(6, 0x18). Both argument roles stay described by that
 * arithmetic because the emitter itself is unlifted.
 */
void func_801F41F0(s16 row, u8 column);

/* Per-step draw-move column table at 0x801F4E70: one byte per step, read with the
 * work record's byte +0x09 divided by three, so each entry covers three frames.
 * The neighbour at 0x801F4E74 carries the same role for this overlay's
 * two-frames-per-step handler at 0x801F4148.
 * @source 0x801F4E70 @kind table
 */
extern u8 moveBlockColumnTable801F4E70[];

/* @source 0x801F40B4
 * @behavior Overlay step handler. It reads the scratchpad work-record pointer
 *           cell 0x1F800044 and divides that record's byte +0x09 by three
 *           (unsigned divide, magic 0xAAAAAAAB, byte-masked) to get the step
 *           index, takes that step's byte from the table at 0x801F4E70 and hands
 *           it, with the record's signed halfword at +0x36, to the draw-move
 *           emitter at 0x801F41F0, which submits one 8 by 0x78 move block. A
 *           second cell read then advances the record's byte +0x09 by one, and a
 *           third cell read shows the advanced byte: once it has reached 4 the
 *           function stores 1 into the record's dispatch state byte +0x02, which
 *           selects entry 1 of the byte-+0x02 handler run based at 0x801F4D80.
 *           The 0x18-byte frame holds the return address at +0x10; the function
 *           takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMoveBlockByWorkByte9ThirdThenIncrementWorkByte9AndSetState2To1At4World01Area04813_801F40B4(
    void) {
  u8* drawWork;
  u8* incWork;
  u8* gateWork;
  u8 step;

  drawWork = workRecordCursor1F800044;
  step = (u8)((u32)drawWork[0x09] / 3);
  func_801F41F0(FIELD_REF(s16, drawWork, 0x36),
                moveBlockColumnTable801F4E70[step]);
  incWork = workRecordCursor1F800044;
  incWork[0x09]++;
  gateWork = workRecordCursor1F800044;
  if (gateWork[0x09] >= 4u) {
    gateWork[0x02] = 1;
  }
}
