#include "bof3/bof3.h"

typedef void (*BmagicMagic09603Handler)(void);

typedef struct BmagicMagic09603DispatchTable {
  BmagicMagic09603Handler handlers[3];
} BmagicMagic09603DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the leading three words this dispatcher
 * copies: entry 0 = 0x801EEC84, entry 1 = 0x801EEDD8, entry 2 = 0x801EEE40, the
 * three code blocks this overlay's Splat declares after its entry point. The run
 * continues past those three words for five more entries (0x801EEFB8,
 * 0x801EF2CC, 0x801EF348, 0x801EF394, 0x801EF3E0) at 0x801EEC10, which the
 * dispatcher at 0x801EEEE4 copies and indexes by work byte 2, so this
 * declaration covers only the prefix this function copies. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is bound
 * by the target's generated undefined-symbol table.
 */
extern BmagicMagic09603DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC24
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before frame creation, its work byte at offset 1 selects an
 *           entry of the three-entry code pointer prefix at 0x801EEC04
 *           (0x801EEC84, 0x801EEDD8, 0x801EEE40), and that entry is invoked with
 *           no arguments. The three table words are copied into this function's
 *           own 0x28-byte frame first (0x10 through 0x18, with the return
 *           address at 0x20), so the call target is read back from the copy and
 *           the selector is not bounds-checked. It reads no other state and
 *           writes none, and no word of this payload holds its own address, so
 *           it is an overlay entry reached from outside the payload; its
 *           prologue and return path alone place the boundary. The 0x60-byte
 *           body is byte-identical to the exact three-entry dispatcher at
 *           0x801EEC28 of emi/bmagic/magic138/03 (one 96-byte payload
 *           comparison, md5 6fbad88f3c0e17c580e1db359900f8db on both spans);
 *           both overlays load at 0x801EEC00 and keep their pointer run at
 *           0x801EEC04, so no immediate shifts between the spans, and the
 *           statement order, operand forms and declaration style were taken
 *           from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Handler24(void) {
  BmagicMagic09603DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
