#include "bof3/bof3.h"

typedef void (*BmagicMagic22203Handler)(void);

typedef struct BmagicMagic22203DispatchTable {
    BmagicMagic22203Handler handlers[3];
} BmagicMagic22203DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC70, entry 1 = 0x801EEE20, entry 2 = 0x801EEE80, the three code spans
 * that follow this overlay's entry point. The run is the `tabledata` bin
 * segment of this target (payload offset 0x04, twelve bytes after the
 * four-byte header word), which no map row owns, so the reference is bound by
 * the target's generated undefined-symbol table.
 */
extern BmagicMagic22203DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/*
 * Seed provenance: this 96-byte span (payload offset 0x10) is byte-identical to
 * the exact lift dispatchWorkByte1HandlerBmagicMagic14503_801EEC10
 * (emi/bmagic/magic145/03, same payload offset), proven with one payload
 * comparison of the two spans. The only words that family member carries
 * differently are its own table contents, which live in the leading data block
 * and not in this body, so the statement order, local shapes and cell spelling
 * are transcribed from that exact sibling and the table entries above are read
 * from this payload's own bytes at offset 0x04.
 */

/* @source 0x801EEC10
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before the frame is created, its state byte at offset 1
 *           selects an entry of the three-entry code pointer table at
 *           0x801EEC04 (0x801EEC70, 0x801EEE20, 0x801EEE80), and that entry is
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
void dispatchWorkByte1HandlerBmagicMagic22203_801EEC10(void) {
    BmagicMagic22203DispatchTable handlers;

    handlers = D_801EEC04;
    handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
