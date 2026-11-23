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

/* @source 0x801F2D5C
 * @behavior Overlay step handler for the projected cursors, reached as entry 0
 *           of the two-entry state-byte-1 handler table based at 0x801F3304: it
 *           loads the work-object pointer published at the scratchpad cell
 *           0x1F800044, passes that object's two cursor words at +0x34 and +0x38
 *           to the shared distance helper func_8015477C, stores the answer
 *           shifted left 16 into the object's third cursor word at +0x3C, and
 *           advances the object's state byte at +1 by one, which hands the step
 *           over to entry 1 of the same table,
 *           renderCursorQuad. The pointer is loaded
 *           before the stack frame because the first argument pair needs it and
 *           nothing before those loads needs the frame, and it is re-read from
 *           the cell after the call rather than kept in a saved register, so the
 *           frame only holds $ra. The state byte is read before the +0x3C store
 *           and written after it, so the handler publishes the new distance
 *           before the dispatch byte that makes entry 1 read it. It takes no
 *           arguments, returns nothing and reads no other state.
 * Provenance: the payload records the table words 0x801F2D5C and 0x801F2DA8 at
 * 0x801F3304, so the entry order and the partner handler above come from this
 * overlay's own bytes. This span compared byte-identical to the lifted span at
 * 0x801F3E58 of BIN/WORLD02/AREA112.EMI#13 (all nineteen words equal, 76 of 76
 * bytes, also reported by the clone dry run), and the statement order, operand
 * forms, offset spelling and declaration style are transcribed from that span's
 * source; the callee and cell bindings are the shared main-executable ones both
 * overlays call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStoreCursorDistance(
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
