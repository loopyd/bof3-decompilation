#include "bof3/bof3.h"

typedef void (*BmagicMagic16903Handler)(void);

typedef struct BmagicMagic16903DispatchTable {
  BmagicMagic16903Handler handlers[6];
} BmagicMagic16903DispatchTable;

/*
 * Code pointer table at 0x801EEC14, the table this dispatcher copies:
 * entry 0 = 0x801EF2CC, entry 1 = 0x801EF444, entry 2 = 0x801EF4FC,
 * entry 3 = 0x801EF640, entry 4 = 0x801EF688, entry 5 = 0x801EF7D0.
 * It is the middle of three back-to-back tables in the overlay's leading data
 * block: 0x801EEC04 holds four entries and 0x801EEC2C the last three, so this
 * declaration only covers the six words this function copies.
 */
extern BmagicMagic16903DispatchTable D_801EEC14; /* @source 0x801EEC14 @kind data */

/* @source 0x801EF254
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its work byte at offset 2 selects an entry of the six-entry code pointer
 * table at 0x801EEC14 (0x801EF2CC, 0x801EF444, 0x801EF4FC, 0x801EF640,
 * 0x801EF688, 0x801EF7D0), and that entry is invoked with no arguments. The
 * six table words are copied into this function's own 0x30-byte frame first,
 * so the call target is read back from the copy. It reads no other state and
 * writes none; the frame only keeps $ra across the indirect call. The
 * preceding 0x68 bytes are byte-identical to the head of the dispatcher at
 * 0x801EF19C of emi/bmagic/magic054/03, which continues with a work-byte-0
 * check this function does not have.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic16903_801EF254(void) {
  BmagicMagic16903DispatchTable handlers;

  handlers = D_801EEC14;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
