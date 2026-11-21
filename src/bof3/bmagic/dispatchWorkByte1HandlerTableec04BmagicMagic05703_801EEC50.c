#include "bof3/bof3.h"

typedef void (*BmagicMagic05703Handler)(void);

typedef struct BmagicMagic05703DispatchTable {
  BmagicMagic05703Handler handlers[3];
} BmagicMagic05703DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EECB0, entry 1 = 0x801EED2C, entry 2 = 0x801EEFEC. It is the first run
 * of code pointers in this overlay, the three words directly after the four-byte
 * header word at 0x801EEC00, and Splat binds it through this target's generated
 * undefined-symbol table.
 */
extern BmagicMagic05703DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC50
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object: the work-object pointer cell at 0x1F800044 is read, its
 *           work byte at offset 1 selects an entry of the three-entry code
 *           pointer table at 0x801EEC04 (0x801EECB0, 0x801EED2C, 0x801EEFEC),
 *           and that entry is invoked with no arguments. The three table words
 *           are copied into this function's own 0x28-byte frame first, so the
 *           call target is read back from the copy. The index is not
 *           bounds-checked. It reads no other state and writes none, and no word
 *           of this payload holds its own address, so it is an overlay entry
 *           reached from outside the payload; its prologue and return path alone
 *           place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerTableec04BmagicMagic05703_801EEC50(void) {
  BmagicMagic05703DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
