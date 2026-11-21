#include "bof3/bof3.h"

typedef void (*BmagicMagic01703Handler)(void);

typedef struct BmagicMagic01703DispatchTable {
  BmagicMagic01703Handler handlers[3];
} BmagicMagic01703DispatchTable;

/*
 * Code pointer run that starts at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC98, entry 1 = 0x801EEF68, entry 2 = 0x801EEFD8, the three
 * phase handlers this overlay's entry point runs. The run continues past those
 * three words with the ten-word step table at 0x801EEC10, so this declaration
 * only covers the prefix this function copies. It sits in the overlay's leading
 * data block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 */
extern BmagicMagic01703DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC38
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * three-entry code pointer run at 0x801EEC04 (0x801EEC98, 0x801EEF68,
 * 0x801EEFD8), and that entry is invoked with no arguments, so no argument
 * register is loaded. The three table words are copied into this function's own
 * 0x28-byte frame first, so the call target is read back from the copy, and the
 * selector is not bounds-checked. It reads no other state and writes none; the
 * frame keeps the copied table and $ra, and nothing is returned. The payload
 * names this address with no jal immediate and no pointer word, so the overlay
 * loader is its only caller and the roles stay structural. The 96-byte span is
 * byte-identical to the three-entry dispatchers at 0x801EEC34 of
 * emi/bmagic/magic039/03, 0x801EEC64 of emi/bmagic/magic001/00 and 0x801EEC10
 * of emi/bmagic/magic048/03, each proved by a payload compare of the two spans.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01703_801EEC38(void) {
  BmagicMagic01703DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
