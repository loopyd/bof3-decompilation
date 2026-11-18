#include "bof3/bof3.h"

typedef void (*BmagicMagic16103Handler)(void);

typedef struct BmagicMagic16103DispatchTable {
  BmagicMagic16103Handler handlers[3];
} BmagicMagic16103DispatchTable;

/*
 * Code pointer run that starts at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC84, entry 1 = 0x801EEE9C, entry 2 = 0x801EEF08, the three
 * code blocks that follow this overlay's entry point. The run continues past
 * those three words with five more overlay handlers (0x801EF024, 0x801EF06C,
 * 0x801EF0E8, 0x801EF158, 0x801EF1BC), so this declaration only covers the
 * prefix this function copies. It sits in the overlay's leading data block,
 * which no map row owns, so the reference is bound by the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic16103DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC24
 * @behavior Dispatches one frame of the overlay work handler from the scratchpad
 *           work object: the work-object pointer cell at 0x1F800044 is read before
 *           frame creation, its work byte at offset 1 selects an entry of the
 *           three-entry code pointer run at 0x801EEC04 (0x801EEC84, 0x801EEE9C,
 *           0x801EEF08), and that entry is invoked with no arguments. The three
 *           table words are copied into this function's own 0x28-byte frame first,
 *           so the call target is read back from the copy and the selector is not
 *           bounds-checked. It reads no other state and writes none, and no word of
 *           this payload holds its own address, so it is an overlay entry reached
 *           from outside the payload; its prologue and return path alone place the
 *           boundary. The 96-byte body is byte-identical to the three-entry
 *           dispatchers at 0x801EEC34 of emi/bmagic/magic039/03, 0x801EEC64 of
 *           emi/bmagic/magic001/00, 0x801EEC50 of emi/bmagic/magic151/03 and
 *           0x801EEC10 of emi/bmagic/magic048/03 (payload byte comparison); those
 *           sources measured exact when this seed was transcribed from the
 *           magic039/03 body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic16103_801EEC24(void) {
  BmagicMagic16103DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
