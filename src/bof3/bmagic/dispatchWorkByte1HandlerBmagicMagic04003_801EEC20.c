#include "bof3/bof3.h"

typedef void (*BmagicMagic04003Handler)(void);

typedef struct BmagicMagic04003DispatchTable {
  BmagicMagic04003Handler handlers[3];
} BmagicMagic04003DispatchTable;

/*
 * Code pointer run at 0x801EEC04, of which this dispatcher copies the leading
 * three words: entry 0 = 0x801EEC80, entry 1 = 0x801EEDFC, entry 2 = 0x801EEEFC,
 * the three overlay-local code blocks that follow this entry point. Four more
 * pointers, 0x801EF074, 0x801EF114, 0x801EF180 and 0x801EF1D0, continue in the
 * same leading data block, so this declaration covers only the prefix the body
 * copies. That block carries no map row, so the reference binds through the
 * target's generated undefined-symbol table.
 */
extern BmagicMagic04003DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC20
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before frame creation, the object's byte at offset 1
 *           selects an entry of the three-entry code pointer run based at
 *           0x801EEC04 (0x801EEC80, 0x801EEDFC, 0x801EEEFC), and that entry is
 *           invoked with no arguments, so no argument register is loaded. The
 *           three table words are copied into the function's own 0x28-byte frame
 *           first, so the call target is read back from the copy and the
 *           selector is not bounds-checked. It reads no other state and writes
 *           none, and the frame keeps only $ra at 0x20. The 0x60-byte span ends
 *           at the return at 0x801EEC7C, where this overlay's next declared body
 *           starts. No word of this payload holds the address 0x801EEC20, so the
 *           caller sits outside the overlay and both parameter roles stay
 *           structural. Comparing this payload's span with the one at
 *           0x801EEC1C of emi/bmagic/magic137/03 measured the 96 bytes equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic04003_801EEC20(void) {
  BmagicMagic04003DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
