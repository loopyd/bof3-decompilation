#include "bof3/bof3.h"

typedef void (*BmagicMagic04103Handler)(void);

typedef struct BmagicMagic04103DispatchTable {
  BmagicMagic04103Handler handlers[6];
} BmagicMagic04103DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the six-entry table this dispatcher
 * copies: entry 0 = 0x801EECB0, entry 1 = 0x801EEE14, entry 2 = 0x801EEEB4,
 * entry 3 = 0x801EEFC4, entry 4 = 0x801EEFF8, entry 5 = 0x801EF130, the six
 * code blocks that follow this overlay's entry point. The run itself starts at
 * 0x801EEC04 and runs up to the entry point at 0x801EEC38 as thirteen words
 * (the last is 0x801EF564), so this declaration only covers the prefix this
 * function copies. The run sits in the overlay's leading data block, which no
 * map row owns, so the reference is bound by the target's generated
 * undefined-symbol table.
 */
extern BmagicMagic04103DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC38
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the six-entry code pointer
 * table at 0x801EEC04 (0x801EECB0, 0x801EEE14, 0x801EEEB4, 0x801EEFC4,
 * 0x801EEFF8, 0x801EF130), and that entry is invoked with no arguments. The six
 * table words are copied into this function's own 0x30-byte frame first
 * (0x10..0x24, with $ra at 0x28), and the byte at offset 1 is read after that
 * copy, so the call target is read back from the copy. It reads no other state
 * and writes none; the three handlers 0x801EEFC4, 0x801EF130 and 0x801EECB0
 * themselves advance the same state byte.
 *
 * Shape: the six-word struct copy into the local table and the indexed call
 * through it copy the statement order, operand forms and declaration style of
 * the six-entry dispatcher at 0x801EEC1C of emi/bmagic/magic086/03, which was
 * written exact. A payload comparison of the two 0x78-byte spans found them
 * byte-identical, and both overlays load at 0x801EEC00 with their pointer run
 * at 0x801EEC04, so the immediates this body materialises are the same words in
 * both images. The type prefix is this target's own; the table entries and the
 * state byte offset are read from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Prefix(void) {
  BmagicMagic04103DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
