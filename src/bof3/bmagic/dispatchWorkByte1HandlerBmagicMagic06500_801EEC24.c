#include "bof3/bof3.h"

typedef void (*BmagicMagic06500Handler)(void);

typedef struct BmagicMagic06500DispatchTable {
  BmagicMagic06500Handler handlers[4];
} BmagicMagic06500DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the four-entry table this dispatcher
 * copies: entry 0 = 0x801EEC90, entry 1 = 0x801EF08C, entry 2 = 0x801EF134,
 * entry 3 = 0x801EF1CC, the four code blocks this overlay's Splat declares after
 * its entry point. The run continues past those four words with four more
 * overlay handlers (0x801EF2E0, 0x801EF328, 0x801EF394, 0x801EF408) at
 * 0x801EEC14, which the dispatcher at 0x801EF250 copies and indexes by work
 * byte 2, so this declaration covers only the prefix this function copies. It
 * sits in the overlay's leading data block, which no map row owns, so the
 * reference is bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic06500DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC24
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before frame creation, its work byte at offset 1 selects an
 *           entry of the four-entry code pointer table at 0x801EEC04
 *           (0x801EEC90, 0x801EF08C, 0x801EF134, 0x801EF1CC), and that entry is
 *           invoked with no arguments. The four table words are copied into
 *           this function's own 0x28-byte frame first (0x10..0x1C, with $ra at
 *           0x20), so the call target is read back from the copy and the
 *           selector is not bounds-checked. It reads no other state and writes
 *           none, and no word of this payload holds its own address, so it is an
 *           overlay entry reached from outside the payload; its prologue and
 *           return path alone place the boundary. The 0x6C-byte body is
 *           byte-identical to the exact four-entry dispatcher at 0x801EEC2C of
 *           emi/bmagic/magic054/03 (one 108-byte payload cmp); both overlays
 *           load at 0x801EEC00 and keep their pointer run at 0x801EEC04, so no
 *           immediate shifts between the spans and the statement order, operand
 *           forms and declaration style are cloned from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic06500_801EEC24(void) {
  BmagicMagic06500DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
