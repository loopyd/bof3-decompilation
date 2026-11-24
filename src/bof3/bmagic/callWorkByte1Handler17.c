#include "bof3/bof3.h"

typedef void (*BmagicMagic06403Handler)(void);

typedef struct BmagicMagic06403DispatchTable {
  BmagicMagic06403Handler handlers[3];
} BmagicMagic06403DispatchTable;

/*
 * Code pointer run that starts at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EECDC, entry 1 = 0x801EECFC, entry 2 = 0x801EF69C. The run
 * continues past those three words, so this declaration only covers the prefix
 * this function copies. It sits in the overlay's leading data block, which no
 * map row owns, so the reference is bound by the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic06403DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC7C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * three-entry code pointer run at 0x801EEC04 (0x801EECDC, 0x801EECFC,
 * 0x801EF69C), and that entry is invoked with no arguments. The three table
 * words are copied into this function's own 0x28-byte frame first, so the call
 * target is read back from the copy and the selector is not bounds-checked. It
 * reads no other state and writes none. The 96-byte body is byte-identical to
 * the three-entry dispatcher dispatchWorkByte1HandlerBmagicMagic03903_801EEC34
 * of emi/bmagic/magic039/03, proved by comparing the two payload spans.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Handler17(void) {
  BmagicMagic06403DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
