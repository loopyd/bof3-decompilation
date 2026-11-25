#include "bof3/bof3.h"

typedef void (*BmagicMagic14503Handler)(void);

typedef struct BmagicMagic14503DispatchTable {
  BmagicMagic14503Handler handlers[3];
} BmagicMagic14503DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC70, entry 1 = 0x801EEDB0, entry 2 = 0x801EEDE4, the three
 * code blocks that follow this overlay's entry point. The run is the
 * `tabledata` bin segment of this target (payload offset 0x04, twelve bytes
 * after the four-byte header word), which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic14503DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/*
 * Seed provenance: this 96-byte span is byte-identical to the exact lifts
 * dispatchWorkByte1HandlerBmagicMagic12003_801EEC10 (emi/bmagic/magic120/03,
 * payload offset 0x10) and
 * dispatchWorkByte1HandlerTableec04BmagicMagic05703_801EEC50
 * (emi/bmagic/magic057/03, payload offset 0x50), proven with one payload
 * comparison each. The only words either family member carries differently are
 * its own table contents, which live in the leading data block and not in this
 * body, so the statement order, local shapes and cell spelling are transcribed
 * from that exact sibling and the table entries above are read from this
 * payload's own bytes at offset 0x04.
 */

/* @source 0x801EEC10
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before the frame is created, its state byte at offset 1
 *           selects an entry of the three-entry code pointer table at
 *           0x801EEC04 (0x801EEC70, 0x801EEDB0, 0x801EEDE4), and that entry is
 *           invoked with no arguments. The three table words are copied into
 *           this function's own 0x28-byte frame first, so the call target is
 *           read back from the copy. The index is not bounds-checked. It reads
 *           no other state and writes none, and no word of this payload holds
 *           its own address, so it is an overlay entry reached from outside the
 *           payload; its prologue and return path alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Slot(void) {
  BmagicMagic14503DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
