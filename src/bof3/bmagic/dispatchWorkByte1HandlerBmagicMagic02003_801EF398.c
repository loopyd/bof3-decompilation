#include "bof3/bof3.h"

typedef void (*BmagicMagic02003Handler)(void);

/* Overlay-local handler run this dispatcher indexes, based at 0x801EFEB8: its
 * three in-image code pointers are 0x801EF3DC, 0x801EF894 and 0x801EFAF4, all
 * function starts of this overlay. The run is the target's trailing data
 * subsegment, which this target's map row owns. */
extern BmagicMagic02003Handler D_801EFEB8[]; /* @source 0x801EFEB8 @kind data */

/* @source 0x801EF398
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the three-entry overlay-local handler run based at
 * 0x801EFEB8 (0x801EF3DC, 0x801EF894, 0x801EFAF4) is invoked with no
 * arguments, so no argument register is loaded. The original does not bound
 * the index before the scaled load. It reads no other state and writes none;
 * the 0x18-byte frame only keeps $ra across the indirect call, and the
 * pointer-cell load precedes frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic02003_801EF398(void) {
  D_801EFEB8[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
