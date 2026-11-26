#include "bof3/bof3.h"

/* Work-record pointer cell in the shared scratchpad, outside every overlay
 * payload. Declared at its fixed address so each read is one lui plus an offset
 * load on the cell, which is how the original re-reads it after the call. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-executable sampler at 0x8015477C: it answers one value for the
 * coordinate pair it is given, and other world overlays of the same executable
 * bind this address as the map-height sampler. This overlay reads the answer as
 * a word and scales it before the store.
 * @source 0x8015477C @kind function
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F2EE4
 * @behavior Entry 0 of the two-entry state-byte-1 step dispatch table based at
 *           0x801F34E8, whose word 0 names this span and whose word 1 names
 *           renderWorkCursorQuad, and which
 *           dispatchByte1Table34e8 indexes with
 *           byte +1 of the work record published at the scratchpad pointer cell
 *           0x1F800044. It loads that work-record pointer, passes the record's
 *           cursor words at +0x34 and +0x38 to the shared main-executable
 *           sampler func_8015477C, stores the answer shifted left 16 into the
 *           record's third cursor word at +0x3C, and then advances byte +1 by
 *           one, which hands the next dispatch to entry 1. The pointer is
 *           loaded before the stack frame because both argument loads need it
 *           and nothing before them needs the frame, and it is read from the
 *           cell again after the call instead of being kept in a saved
 *           register, so the 0x18-byte frame holds only $ra and the call has a
 *           nop delay slot. Byte +1 is read before the +0x3C store and written
 *           after it, so the new cursor word is published before the byte that
 *           makes entry 1 read it. The dispatcher sets up no argument register,
 *           so the entry takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCursorDistanceAdvance(
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
