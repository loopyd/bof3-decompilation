#include "bof3/bof3.h"

typedef void (*BmagicMagic05703StageHandler)(void);

typedef struct BmagicMagic05703StageDispatchTable {
  BmagicMagic05703StageHandler handlers[4];
} BmagicMagic05703StageDispatchTable;

/*
 * Code pointer table at 0x801EEC10, the table this dispatcher copies: entry 0 =
 * 0x801EED98, entry 1 = 0x801EEE50, entry 2 = 0x801EEEB0, entry 3 = 0x801EEFA4.
 * It is the second run of code pointers in this overlay, the four words directly
 * after the three-entry work-byte-1 table at 0x801EEC04, and Splat binds it
 * through this target's generated undefined-symbol table.
 */
extern BmagicMagic05703StageDispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* @source 0x801EED2C
 * @behavior Second handler of the three-entry work-byte-1 handler table at
 *           0x801EEC04, which the overlay entry dispatcher
 *           dispatchWorkByte1HandlerTableec04BmagicMagic05703_801EEC50 indexes
 *           with the scratchpad work object unsigned byte 0x01, so no jal in the
 *           image names this span. It reads the work-object pointer cell at
 *           0x1F800044 once, before it reserves its frame, materialises the
 *           table base 0x801EEC10 once and copies the four words there into its
 *           own 0x28-byte frame at 0x10, 0x14, 0x18 and 0x1C as bare load and
 *           store pairs. Last it reads work byte 0x02 through the pointer it
 *           already holds, scales it by four, adds the frame pointer and invokes
 *           the copied entry with no arguments; the index is not bounds-checked
 *           and that byte is the handler index the state-2 dispatcher
 *           dispatchWorkByte2HandlerTableec38BmagicMagic05703_801EF55C also
 *           reads. It reads no other state, writes none, takes no arguments and
 *           returns nothing through the jr $ra at 0x801EED90 inside the
 *           0x28-byte frame that keeps $ra at 0x20.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerTableec10BmagicMagic05703_801EED2C(void) {
  BmagicMagic05703StageDispatchTable handlers;

  handlers = D_801EEC10;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
