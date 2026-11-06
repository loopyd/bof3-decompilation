#include "bof3/bof3.h"

typedef void (*BmagicMagic09903Handler)(void);

typedef struct BmagicMagic09903DispatchTable {
  BmagicMagic09903Handler handlers[5];
} BmagicMagic09903DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC88, entry 1 = 0x801EEE70, entry 2 = 0x801EEF98, entry 3 = 0x801EEFDC,
 * entry 4 = 0x801EF0EC, the five code blocks that follow this overlay's entry
 * point. The run sits in the overlay's leading data block, which no map row
 * owns, so the reference is bound by the target's generated undefined-symbol
 * table.
 */
extern BmagicMagic09903DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC18
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object: the work-object pointer cell at 0x1F800044 is read, its
 *           state byte at offset 1 selects an entry of the five-entry code
 *           pointer table at 0x801EEC04 (0x801EEC88, 0x801EEE70, 0x801EEF98,
 *           0x801EEFDC, 0x801EF0EC), and that entry is invoked with no
 *           arguments. The five table words are copied into this function's own
 *           0x30-byte frame first, so the call target is read back from the
 *           copy. It reads no other state and writes none, and no word of this
 *           payload holds its own address, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place
 *           the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic09903_801EEC18(void) {
  BmagicMagic09903DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
