#include "bof3/bof3.h"

/* Shared front-end mode byte of the shared main executable at 0x80143BB0. The
 * body reads it once through its symbol expression, the `lui`/`lbu` pair the
 * original bytes show, and tests it for 3. Exact lifts across the corpus bind
 * this cell address-anchored, and the shared map carries the address. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */

/* @source 0x801F2C48
 * @behavior Overlay step handler that runs as entry 0 of the state-byte-1
 *           handler run based at 0x801F2E44, whose first word is this span and
 *           which the overlay dispatcher at 0x801F2C04 indexes with the work
 *           record byte at offset 0x01 scaled by four. When the shared
 *           front-end mode byte at 0x80143BB0 reads exactly 3 it loads the work
 *           record published at the scratchpad pointer cell 0x1F800044 and
 *           stores 1 into that record's byte at offset 0x01, so state 0 of that
 *           run hands the record on to state 1. The compared 3 is materialized
 *           before the branch and the stored 1 fills that branch's delay slot,
 *           and the pointer cell is read only on the storing path, so the
 *           original keeps no local and no frame. Any other mode byte value
 *           leaves the record untouched. It takes no arguments, makes no call,
 *           reads no state besides the mode byte and the cell, and its 0x2C
 *           bytes carry no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchWorkState1WhenFrontEndModeIs3World02Area10713_801F2C48(void) {
  if (D_80143BB0 == 3) {
    SPAD_PTR_SLOT(u8, 0x44u)[1] = 1;
  }
}
