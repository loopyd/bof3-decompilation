#include "bof3/bof3.h"

typedef void (*BmagicMagic13703Handler)(void);

typedef struct BmagicMagic13703DispatchTable {
  BmagicMagic13703Handler handlers[3];
} BmagicMagic13703DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the three-word table this dispatcher
 * copies: entry 0 = 0x801EEC7C, entry 1 = 0x801EEEFC, entry 2 = 0x801EEF64, the
 * three code blocks that follow this overlay's entry point. The three-word
 * table at 0x801EEC10, copied by the work-byte-2 dispatcher at 0x801EFFBC,
 * follows immediately. The run sits in the overlay's leading data block, which
 * no map row owns, so the reference binds through the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic13703DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC1C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its state byte at offset 1 selects an entry of the
 * three-entry code pointer table at 0x801EEC04 (0x801EEC7C, 0x801EEEFC,
 * 0x801EEF64), and that entry is invoked with no arguments. The three table
 * words are copied into its own 0x28-byte frame first, so the call target is
 * read back from the copy and the selector is not bounds-checked. It reads no
 * other state and writes none. It takes no arguments, returns nothing and its
 * 0x28-byte frame keeps only $ra. No word of this payload holds its own
 * address, so it is an overlay entry reached from outside the payload; its
 * prologue and return path alone place the boundary. The 96-byte body is
 * byte-identical to the three-entry dispatcher at 0x801EEC28 of
 * emi/bmagic/magic138/03, proved by comparing this payload's span with that
 * one; it differs from the work-byte-2 dispatcher at 0x801EFFBC of this target
 * in the copied table's low word and in the record byte it indexes alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic13703_801EEC1C(void) {
  BmagicMagic13703DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
