#include "bof3/bof3.h"

typedef void (*BmagicMagic15103Handler)(void);

typedef struct BmagicMagic15103DispatchTable {
  BmagicMagic15103Handler handlers[5];
} BmagicMagic15103DispatchTable;

/*
 * Code pointer table at 0x801EEC24, the table this dispatcher copies:
 * entry 0 = 0x801EEEA8, entry 1 = 0x801EEF24, entry 2 = 0x801EF388,
 * entry 3 = 0x801EF484, entry 4 = 0x801EF2A0. The run sits in the overlay's
 * leading data block, which no map row owns, so the reference is bound by the
 * target's generated undefined-symbol table.
 */
extern BmagicMagic15103DispatchTable D_801EEC24; /* @source 0x801EEC24 @kind data */

/* @source 0x801EF318
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 * uses the work object's unsigned byte at offset 3 to select an entry of the
 * five-entry code pointer table at 0x801EEC24. The five table words are copied
 * into this function's own 0x30-byte frame first, so the call target is read
 * back from the copy, and that entry is invoked with no arguments. The original
 * does not check the index and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte3HandlerBmagicMagic15103_801EF318(void) {
  BmagicMagic15103DispatchTable handlers;

  handlers = D_801EEC24;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[3]]();
}
