#include "bof3/bof3.h"

typedef void (*BmagicMagic10303Handler)(void);

typedef struct BmagicMagic10303DispatchTable {
  BmagicMagic10303Handler handlers[3];
} BmagicMagic10303DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC70, entry 1 = 0x801EEE38, entry 2 = 0x801EEE94. It is the first run
 * of code pointers in this overlay, the three words directly after the four-byte
 * header word at 0x801EEC00, and Splat binds it through this target's generated
 * undefined-symbol table.
 */
extern BmagicMagic10303DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC10
 * @behavior Overlay step dispatcher and the first body of this payload. It reads
 *           the scratchpad work-object pointer cell at 0x1F800044 once, copies
 *           the three-entry code pointer table at 0x801EEC04 (0x801EEC70,
 *           0x801EEE38, 0x801EEE94) into a three-word local handler array in its
 *           own 0x28-byte frame, and invokes the entry selected by the work
 *           object's unsigned byte 0x01 with no arguments. The table words are
 *           read back from the copy, the index is not bounds-checked, nothing of
 *           the selected step's result is returned, and the frame only keeps $ra
 *           at 0x20 across the indirect call. No jal and no aligned pointer word
 *           in the payload names 0x801EEC10, so the overlay loader is its only
 *           caller and the body takes no arguments. Statement order, operand
 *           forms and declaration style follow the exact lift
 *           dispatchWorkByte1HandlerTableec04BmagicMagic05703_801EEC50: a cmp of
 *           this payload's 0x60 bytes at offset 0x10 against that target's 0x60
 *           bytes at offset 0x50 is equal word for word, so the 0x28 frame, the
 *           lui/lw pair of the scratchpad cell, the three table loads, the three
 *           stack stores, the scaled stack indexing and the jalr are one body.
 *           The table's three addresses are this payload's own bytes and the
 *           table is read through the fixed cell address, so each target's own
 *           handler addresses stay local.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkHandlerTableec04(void) {
  BmagicMagic10303DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
