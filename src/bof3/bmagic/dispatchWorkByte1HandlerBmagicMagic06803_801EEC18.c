#include "bof3/bof3.h"

typedef void (*BmagicMagic06803Handler)(void);

typedef struct BmagicMagic06803DispatchTable {
  BmagicMagic06803Handler handlers[5];
} BmagicMagic06803DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC88, entry 1 = 0x801EED50, entry 2 = 0x801EF44C, entry 3 = 0x801EF4A8
 * and entry 4 = 0x801EF4EC, the five code blocks that follow this overlay's
 * entry point. The run sits in the overlay's leading data block, which no map
 * row owns, so the reference is bound by the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic06803DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC18
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object: the work-object pointer cell at 0x1F800044 is read, its
 *           state byte at offset 1 selects an entry of the five-entry code
 *           pointer table at 0x801EEC04 (0x801EEC88, 0x801EED50, 0x801EF44C,
 *           0x801EF4A8, 0x801EF4EC), and that entry is invoked with no
 *           arguments. The five table words are copied into this function's own
 *           0x30-byte frame first, three loads and three stores followed by two
 *           and two, so the call target is read back from the copy at
 *           sp+0x10+index*4 and the index is not bounds-checked. It reads no
 *           other state, writes none and returns nothing, and no word of this
 *           payload holds its own address, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place
 *           the boundary. This target reaches work byte 1 a second time from
 *           0x801EF73C, which indexes the tail run at 0x801EFCC8 in place instead
 *           of copying it, so the copy of the leading run is what this span
 *           names. The 0x70-byte body measured word for word equal to the
 *           dispatchers at 0x801EEC18 of emi/bmagic/magic099/03, at 0x801EEC28
 *           of emi/bmagic/magic009/03 and of emi/bmagic/magic004/03, and at
 *           0x801EEC24 of emi/bmagic/magic067/03, each lifted exact in its own
 *           target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic06803_801EEC18(void) {
  BmagicMagic06803DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
