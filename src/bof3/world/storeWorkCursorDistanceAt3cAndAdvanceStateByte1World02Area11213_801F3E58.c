#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair as its two coordinates and answers the distance as a halfword, which this
 * overlay reads as a word.
 * @source 0x8015477C @kind function
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F3E58
 * @behavior Overlay step handler for the projected cursors, reached as entry 0
 *           of the state-byte-1 handler table based at 0x801F493C: it loads the
 *           work-object pointer published at the scratchpad cell 0x1F800044,
 *           passes that object's two cursor words at +0x34 and +0x38 to the
 *           shared distance helper func_8015477C, stores the answer shifted left
 *           16 into the object's third cursor word at +0x3C, and advances the
 *           object's state byte at +1 by one, which hands the step over to
 *           entry 1 of the same table, drawWorkCursorQuadWorld02Area11213_801F3EA4.
 *           The pointer is loaded before the stack frame because the first
 *           argument pair needs it and nothing before those loads needs the
 *           frame, and it is re-read from the cell after the call rather than
 *           kept in a saved register, so the frame only holds $ra. The state
 *           byte is read before the +0x3C store and written after it, so the
 *           handler publishes the new distance before the dispatch byte that
 *           makes entry 1 read it. It takes no arguments, returns nothing and
 *           reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeWorkCursorDistanceAt3cAndAdvanceStateByte1World02Area11213_801F3E58(
    void) {
  u8 *work;
  u8 *next;
  u8 state;
  s32 distance;

  work = D_1F800044;
  distance = func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38));
  next = D_1F800044;
  state = next[1];
  *(s32 *)(next + 0x3C) = distance << 16;
  next[1] = state + 1;
}
