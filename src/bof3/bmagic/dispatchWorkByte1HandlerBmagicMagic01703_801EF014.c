#include "bof3/bof3.h"

typedef void (*BmagicMagic01703Handler)(void);

/* Overlay-local handler run this dispatcher indexes, based at 0x801EF990: the
 * only word of the run inside this payload is 0x801EF058, the function start
 * of the sibling dispatcher dispatchWorkByte1HandlerBmagicMagic00800_801EF058.
 * The run is the target's trailing 4-byte data subsegment, which this target's
 * map row owns. */
extern BmagicMagic01703Handler func_801EF990[]; /* @source 0x801EF990 @kind data */

/* @source 0x801EF014
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the overlay-local handler run based at 0x801EF990 is
 * invoked with no arguments, so no argument register is loaded. The original
 * does not bound the index before the scaled load. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call, and
 * the pointer-cell load precedes frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01703_801EF014(void) {
  func_801EF990[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
