#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800DispatchTable {
  BmagicMagic00800Handler handlers[4];
} BmagicMagic00800DispatchTable;

/*
 * Code pointer table at 0x801EEC84, the table this dispatcher copies:
 * entry 0 = 0x801F09D0, entry 1 = 0x801F0A20, entry 2 = 0x801F0AC8,
 * entry 3 = 0x801F0B44.
 */
extern BmagicMagic00800DispatchTable D_801EEC84; /* @source 0x801EEC84 @kind data */

/* @source 0x801F0964
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its byte at offset 2 selects an entry of the four-entry code pointer table at
 * 0x801EEC84 (0x801F09D0, 0x801F0A20, 0x801F0AC8, 0x801F0B44), and that entry
 * is invoked with no arguments. As at 0x801EECF0, 0x801EF384, 0x801EF75C and
 * 0x801F0250, the four table words are copied into this dispatcher's own
 * 0x28-byte frame first, so the call target is read back from the copy. It
 * reads no other state and writes none. The entry selected by work byte 2 is
 * the same state-2 slot that the six-entry dispatcher at 0x801F0250 publishes
 * for state byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic00800_801F0964(void) {
  BmagicMagic00800DispatchTable handlers;

  handlers = D_801EEC84;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
