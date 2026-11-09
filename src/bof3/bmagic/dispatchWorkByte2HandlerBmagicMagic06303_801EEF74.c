#include "bof3/bof3.h"

typedef void (*BmagicMagic06303Handler)(void);

typedef struct BmagicMagic06303DispatchTable {
  BmagicMagic06303Handler handlers[5];
} BmagicMagic06303DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEFE4, entry 1 = 0x801EF110, entry 2 = 0x801EF190, entry 3 = 0x801EF1C8,
 * entry 4 = 0x801EF2E4, the five code blocks that follow this overlay's five
 * leading steps. The run fills the overlay's whole leading data block
 * (0x801EEC04..0x801EEC17), which no map row owns, so the reference binds
 * through the target's generated undefined-symbol table.
 */
extern BmagicMagic06303DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEF74
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object: the work-object pointer cell at 0x1F800044 is read, its
 *           work byte at offset 2 selects an entry of the five-entry code
 *           pointer table at 0x801EEC04 (0x801EEFE4, 0x801EF110, 0x801EF190,
 *           0x801EF1C8, 0x801EF2E4), and that entry is invoked with no
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
void dispatchWorkByte2HandlerBmagicMagic06303_801EEF74(void) {
  BmagicMagic06303DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
