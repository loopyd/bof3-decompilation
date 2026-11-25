#include "bof3/bof3.h"

typedef void (*BmagicMagic11103Handler)(void);

typedef struct BmagicMagic11103DispatchTable {
  BmagicMagic11103Handler handlers[3];
} BmagicMagic11103DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the three-word table this dispatcher
 * copies: entry 0 = 0x801EEC7C, entry 1 = 0x801EEF44, entry 2 = 0x801EEFC4, the
 * three code blocks that follow this overlay's entry point. The overlay's
 * leading data block runs six words from 0x801EEC04 to 0x801EEC1B and continues
 * with 0x801EF0C0, 0x801EF0F0 and 0x801EF138; this dispatcher copies only the
 * first three. The run sits in that leading data block, which no map row owns,
 * so the reference binds through the target's generated undefined-symbol table.
 */
extern BmagicMagic11103DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC1C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * three-entry code pointer table at 0x801EEC04 (0x801EEC7C, 0x801EEF44,
 * 0x801EEFC4), and that entry is invoked with no arguments. The three table
 * words are copied into this function's own 0x28-byte frame first, so the call
 * target is read back from the copy and the selector is not bounds-checked. It
 * reads no other state and writes none. Takes no arguments and returns nothing.
 * No word of this payload holds its own address, so it is an overlay entry
 * reached from outside the payload; its prologue and return path alone place
 * the boundary. The 96-byte body is byte-identical to the three-entry
 * dispatchers at 0x801EEC1C of emi/bmagic/magic137/03 and at 0x801EEC28 of
 * emi/bmagic/magic138/03, proved by comparing this payload's span with theirs
 * word for word, and its statement order and operand forms follow those
 * lifts.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByte1Copy(void) {
  BmagicMagic11103DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
