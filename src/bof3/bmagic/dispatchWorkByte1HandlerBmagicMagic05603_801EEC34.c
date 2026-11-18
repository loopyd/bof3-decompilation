#include "bof3/bof3.h"

typedef void (*BmagicMagic05603Handler)(void);

typedef struct BmagicMagic05603DispatchTable {
  BmagicMagic05603Handler handlers[3];
} BmagicMagic05603DispatchTable;

/*
 * Code pointer run that starts at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC94, entry 1 = 0x801EEF3C, entry 2 = 0x801EF17C, the three
 * handler blocks that follow this overlay's entry point. The nine-word run at
 * 0x801EEC10 continues past those three words, so this declaration only covers
 * the prefix this function copies. It sits in the overlay's leading data block,
 * which no map row owns, so the reference is bound by the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic05603DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC34
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * three-entry code pointer run at 0x801EEC04 (0x801EEC94, 0x801EEF3C,
 * 0x801EF17C), and that entry is invoked with no arguments. The three table
 * words are copied into this function's own 0x28-byte frame first, so the call
 * target is read back from the copy and the selector is not bounds-checked. It
 * reads no other state and writes none, and the 96-byte body compared equal
 * word for word to the dispatcher at 0x801EEC34 of emi/bmagic/magic039/03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic05603_801EEC34(void) {
  BmagicMagic05603DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
