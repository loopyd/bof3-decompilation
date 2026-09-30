#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800DispatchTable {
  BmagicMagic00800Handler handlers[4];
} BmagicMagic00800DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EED5C, entry 1 = 0x801EEF6C, entry 2 = 0x801EEFDC,
 * entry 3 = 0x801EF010.
 */
extern BmagicMagic00800DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EECF0
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the four-entry code pointer
 * table at 0x801EEC04 (0x801EED5C, 0x801EEF6C, 0x801EEFDC, 0x801EF010), and
 * that entry is invoked with no arguments. The four table words are copied into
 * this function's own 0x28-byte frame first, so the call target is read back
 * from the copy; it differs from the sibling dispatchers at 0x801EF058,
 * 0x801EFA74, 0x801EFD30 and 0x801F0250 only in that table and entry count. It
 * reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00800_801EECF0(void) {
  BmagicMagic00800DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
