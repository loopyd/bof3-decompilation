#include "bof3/bof3.h"

typedef void (*BmagicMagic06000Handler)(void);

typedef struct BmagicMagic06000DispatchTable {
  BmagicMagic06000Handler handlers[5];
} BmagicMagic06000DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EECA4, entry 1 = 0x801EED68, entry 2 = 0x801EEE00, entry 3 = 0x801EEE84,
 * entry 4 = 0x801EEF04, the five work-byte-1 step handlers of this overlay, with
 * the run terminated by a zero word at 0x801EEC18. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is bound
 * by the target's generated undefined-symbol table.
 */
extern BmagicMagic06000DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it once, with the frame pointer still untouched, and reads
 * the object's unsigned byte 0x01 as the step selector. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEC34
 * @behavior Work-byte-1 step dispatcher of this overlay and its entry point: the
 *           scratchpad work-object pointer cell at 0x1F800044 is read before the
 *           frame exists, its unsigned work byte at offset 1 selects an entry of
 *           the five-entry code pointer run at 0x801EEC04 (0x801EECA4,
 *           0x801EED68, 0x801EEE00, 0x801EEE84, 0x801EEF04), and that entry is
 *           invoked with no arguments. The five table words are copied into this
 *           function's own 0x30-byte frame first, so the call target is read back
 *           from the copy at sp+0x10+index*4 and the selector is not
 *           bounds-checked. It writes nothing else and returns nothing, and no
 *           word of this payload holds its own address, so it is an overlay entry
 *           reached from outside the payload; its prologue and return path alone
 *           place the boundary. The 0x70-byte body is byte-identical to the
 *           lifted work-byte-1 dispatchers of emi/bmagic/magic099/03 at
 *           0x801EEC18, emi/bmagic/magic067/03 at 0x801EEC24, emi/bmagic/magic004/03
 *           at 0x801EEC28, emi/bmagic/magic009/03 at 0x801EEC28,
 *           emi/bmagic/magic068/03 at 0x801EEC18 and emi/bmagic/magic132/03 at
 *           0x801EEC18, so its statement order, operand forms and declaration
 *           style follow that single body; the handler addresses it copies are
 *           this payload's own and are written from these bytes. The roles of
 *           work byte 1 and of the five handlers stay unestablished, so the byte
 *           offset and the table address anchor the name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte1Handler15(void) {
  BmagicMagic06000DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
}
