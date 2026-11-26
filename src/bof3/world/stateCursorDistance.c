#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable helper. This caller passes two coordinate words and
 * consumes the returned word before shifting its bits left by 16.
 * @source 0x8015477C @kind function
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F3014
 * @behavior Overlay step handler for the projected cursor of the work object,
 *           reached as entry 0 of the two-entry state-byte-1 handler table based
 *           at 0x801F3824: it loads the work-object pointer published at the
 *           scratchpad cell 0x1F800044, passes that object's two position words
 *           at +0x34 and +0x38 to the shared distance helper func_8015477C,
 *           stores the answer shifted left 16 into the object's third triple word
 *           at +0x3C, and advances the object's state byte at +1 by one, which
 *           hands the step to entry 1 of the same table,
 *           passDistanceQuadEmitter,
 *           the step that copies +0x34, +0x38 and +0x3C unchanged into a
 *           three-word stack triple and hands that triple to the overlay quad
 *           emitter drawProjectedQuad. The pointer is loaded before the stack frame
 *           because the first argument pair needs it and nothing before those
 *           loads needs the frame, and it is re-read from the cell after the call
 *           rather than kept in a saved register, so the frame only holds $ra.
 *           The state byte is read before the +0x3C store and written after it,
 *           so the handler publishes the new distance before the dispatch byte
 *           that makes entry 1 read it. It takes no arguments, returns nothing
 *           and reads no other state.
 * Provenance: this payload records the table words 0x801F3014 and 0x801F3060 at
 * 0x801F3824, and the dispatcher body at 0x801F2FD0 (src/bof3/world/area05913DispatchTables.c)
 * indexes that table with the work object's byte +1, so the entry order and the
 * partner handler above come from this overlay's own bytes. This span compared
 * byte-identical to the span at 0x801F2D5C of BIN/WORLD00/AREA036.EMI#13 (all
 * nineteen words equal, 76 of 76 bytes; the clone dry run against that
 * representative reported the same), and the statement order, operand forms,
 * offset spelling and declaration style are transcribed from that span's source;
 * the callee and the scratchpad cell are the shared main-executable words both
 * overlays address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCursorDistance(
    void) {
  u8 *work;
  u8 *next;
  u8 state;
  s32 distance;

  work = D_1F800044;
  distance = func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38));
  next = D_1F800044;
  state = next[1];
  *(u32 *)(next + 0x3C) = (u32)distance << 16;
  next[1] = state + 1;
}
