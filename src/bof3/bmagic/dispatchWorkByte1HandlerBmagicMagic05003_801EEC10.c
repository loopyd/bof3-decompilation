#include "bof3/bof3.h"

typedef void (*BmagicMagic05003Handler)(void);

typedef struct BmagicMagic05003DispatchTable {
  BmagicMagic05003Handler handlers[3];
} BmagicMagic05003DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC70, entry 1 = 0x801EEDF8, entry 2 = 0x801EEEA8.
 */
extern BmagicMagic05003DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC10
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC04 (0x801EEC70, 0x801EEDF8, 0x801EEEA8), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy; it reads no other state and writes none. The 96-byte body is
 * instruction-for-instruction identical to the work-byte-1 dispatchers of
 * emi/bmagic/magic008/00 (0x801EF058 and 0x801EECF0's shorter sibling shape),
 * differing only in the %lo table-address immediate of the copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic05003_801EEC10(void) {
  BmagicMagic05003DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
