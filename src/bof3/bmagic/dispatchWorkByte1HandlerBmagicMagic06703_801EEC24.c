#include "bof3/bof3.h"

typedef void (*BmagicMagic06703LeadHandler)(void);

typedef struct BmagicMagic06703LeadDispatchTable {
  BmagicMagic06703LeadHandler handlers[5];
} BmagicMagic06703LeadDispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC94, entry 1 = 0x801EEF3C, entry 2 = 0x801EEFA4,
 * entry 3 = 0x801EF010, entry 4 = 0x801EF284, the five code blocks that follow
 * this overlay's entry point. The run continues at 0x801EEC18 with the three
 * state-2 handlers this overlay's dispatcher at 0x801F038C copies, so this
 * declaration covers only the prefix this function copies. It sits in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic06703LeadDispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC24
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before the frame is created, its unsigned work byte at offset 1 selects an
 * entry of the five-entry code pointer run at 0x801EEC04 (0x801EEC94,
 * 0x801EEF3C, 0x801EEFA4, 0x801EF010, 0x801EF284), and that entry is invoked
 * with no arguments. The five table words are copied into this function's own
 * 0x30-byte frame first, so the call target is read back from the copy at
 * sp+0x10+index*4 and the selector is not bounds-checked. It reads no other
 * state, writes none and returns nothing, and no word of this payload holds its
 * own address, so its prologue and return path alone place the boundary. The
 * 0x70-byte body compared equal word for word to the dispatcher at 0x801EEC18
 * of emi/bmagic/magic099/03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic06703_801EEC24(void) {
  BmagicMagic06703LeadDispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
