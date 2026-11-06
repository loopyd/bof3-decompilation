#include "bof3/bof3.h"

typedef void (*BmagicMagic03903Handler)(void);

typedef struct BmagicMagic03903DispatchTable {
  BmagicMagic03903Handler handlers[3];
} BmagicMagic03903DispatchTable;

/*
 * Code pointer run that starts at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC94, entry 1 = 0x801EEEA8, entry 2 = 0x801EEEDC, the three
 * code blocks that follow this overlay's entry point. The run continues past
 * those three words, so this declaration only covers the prefix this function
 * copies. It sits in the overlay's leading data block, which no map row owns,
 * so the reference is bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic03903DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC34
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * three-entry code pointer run at 0x801EEC04 (0x801EEC94, 0x801EEEA8,
 * 0x801EEEDC), and that entry is invoked with no arguments. The three table
 * words are copied into this function's own 0x28-byte frame first, so the call
 * target is read back from the copy and the selector is not bounds-checked. It
 * reads no other state and writes none, and the 96-byte body is byte-identical
 * to the three-entry dispatcher at 0x801EEC64 of emi/bmagic/magic001/00.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic03903_801EEC34(void) {
  BmagicMagic03903DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
