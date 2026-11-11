#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler keep that base in a saved register across
 * the call and load through offset zero instead, which costs the frame slot.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Two-frame draw-move column table at 0x801F4E74: four bytes, read with the
 * work record's byte +0x09 halved, so each entry covers two frames of this
 * handler's seven-tick cycle. It sits four bytes past the three-frame table
 * 0x801F4E70 that the sibling handler at 0x801F40B4 divides by three, and four
 * past the table 0x801F4E6C that the sibling at 0x801F3EE4 halves.
 * @source 0x801F4E74 @kind table
 */
extern u8 moveBlockTwoFrameColumnTable801F4E74[];

/* Draw-move emitter at 0x801F41F0, shared by this overlay's three move-block
 * handlers. Its own bytes take a grid row index and an 8-pixel column index,
 * build the 0x8 by 0x78 rect x = 0x240 + row * 0x40 + (column + 1) * 8,
 * y = 0x160 at +0x10 of its frame, hand that rect to SetDrawMove with the
 * primitive published in g_PrimCursor and then call func_8014E5A0(6, 0x18);
 * both argument roles stay described by that arithmetic because the emitter is
 * unlifted.
 * @source 0x801F41F0
 */
extern void func_801F41F0(s16 row, u8 column);

/* Per-row draw routine at 0x801F4258, unlifted. Its own bytes index the byte
 * tables at 0x801F4E78, 0x801F4E7C, 0x801F4E80 and 0x801F4E84 with the grid row
 * its first argument carries, then submit that row's primitives through
 * RotTransPers, SetDrawMode and SetSemiTrans, so the argument stays a row index
 * by construction and nothing else is claimed about the body.
 * @source 0x801F4258
 */
extern void func_801F4258(s16 row);

/* @source 0x801F4148
 * @behavior Overlay step handler: entry 5, the last of the six-handler run
 *           based at 0x801F4E54 that this target's byte-+0x02 dispatcher at
 *           0x801F3D60 selects, so no jal reaches this body. It reads the
 *           scratchpad work-record pointer cell 0x1F800044 and halves that
 *           record's byte +0x09 (srl by 1, no byte mask) to get the step index,
 *           takes that step's byte from the table at 0x801F4E74 and hands it,
 *           with the record's signed halfword at +0x36, to the draw-move emitter
 *           at 0x801F41F0, which submits one 8 by 0x78 move block. A second cell
 *           read then advances the record's byte +0x09 by one, and a third cell
 *           read gates the advanced byte: once it has reached 7 the function
 *           clears it and, through a fourth cell read, stores 3 into the
 *           record's dispatch state byte +0x02, which selects entry 3 of the
 *           same handler run for the next frame. A fifth cell read then passes
 *           the record's halfword +0x36 to the per-row draw routine at
 *           0x801F4258. The 0x18-byte frame holds the return address at +0x10;
 *           the function takes no arguments and returns nothing.
 *
 *           Name evidence: this body is the two-frames-per-step member of the
 *           move-block handler family, so its name mirrors the sibling lifted
 *           from the same table run,
 *           drawMoveBlockByWorkByte9ThirdThenIncrementWorkByte9AndSetState2To1At4World01Area04813_801F40B4
 *           ("Third", "/ 3"), and it draws one move block from the work byte's
 *           halved step ("Half", "/ 2"). The sibling's own comment records that
 *           0x801F4E74 carries the same column-table role for this overlay's
 *           two-frames-per-step handler at 0x801F4148, which is this body. The
 *           trailing row draw is named from its own bytes, whose row argument
 *           indexes the per-row tables at 0x801F4E78..0x801F4E84. Roles this
 *           span does not establish stay structural: the emitter's two
 *           arguments, the row routine's single argument and the four tables
 *           are described by their arithmetic only.
 *
 *           Shape evidence: the statement order, the pointer-local spelling for
 *           the scratchpad cell and the reload grouping come from the exact
 *           sibling at 0x801F40B4 of this payload; the / 2 step, the
 *           clear-then-reload arm and the trailing row call come from the
 *           unlifted twin at 0x801F3EE4 of the same payload, whose own bytes
 *           show that shape. Every literal here is this span's own: the halving
 *           shift, the table base 0x801F4E74, the bound 7 and the arm's 3.
 *           Clone was declined on measured evidence: a scan of every payload
 *           under out/binaries for this span's 28-byte address-free prefix found
 *           it only at 0x801F3EE4 and 0x801F4148 of this payload, and those two
 *           spans differ at the table base 0x4E6C/0x4E74 and at the bound
 *           5/7, which are data immediates, so no byte-identical twin exists and
 *           the in-target twin is not a clone representative anyway. The data
 *           lever was declined by the target's empty data-candidate list and the
 *           carve lever by a span that opens on its own entry and ends at the
 *           next declared one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMoveBlockByWorkByte9HalfThenIncrementWorkByte9WrapAt7ToState2Is3AndDrawRowWorld01Area04813_801F4148(
    void) {
  u8* drawWork;
  u8* incWork;
  u8* gateWork;
  u8* resetWork;
  u8* rowWork;
  u8 step;

  drawWork = workRecordCursor1F800044;
  step = (u8)((u32)drawWork[0x09] / 2);
  func_801F41F0(FIELD_REF(s16, drawWork, 0x36),
                moveBlockTwoFrameColumnTable801F4E74[step]);
  incWork = workRecordCursor1F800044;
  incWork[0x09]++;
  gateWork = workRecordCursor1F800044;
  if (gateWork[0x09] >= 7u) {
    gateWork[0x09] = 0;
    resetWork = workRecordCursor1F800044;
    resetWork[0x02] = 3;
  }
  rowWork = workRecordCursor1F800044;
  func_801F4258(FIELD_REF(s16, rowWork, 0x36));
}
