#include "bof3/bof3.h"

typedef void (*BmagicMagic08603Handler)(void);

typedef struct BmagicMagic08603DispatchTable {
  BmagicMagic08603Handler handlers[6];
} BmagicMagic08603DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the six-entry table this dispatcher
 * copies: entry 0 = 0x801EEC94, entry 1 = 0x801EEE44, entry 2 = 0x801EEEE0,
 * entry 3 = 0x801EEFF0, entry 4 = 0x801EF024, entry 5 = 0x801EF168, the six
 * code blocks that follow this overlay's entry point. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic08603DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC1C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the six-entry code pointer
 * table at 0x801EEC04 (0x801EEC94, 0x801EEE44, 0x801EEEE0, 0x801EEFF0,
 * 0x801EF024, 0x801EF168), and that entry is invoked with no arguments. The six
 * table words are copied into this function's own 0x30-byte frame first, so the
 * call target is read back from the copy. It reads no other state and writes
 * none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic08603_801EEC1C(void) {
  BmagicMagic08603DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
