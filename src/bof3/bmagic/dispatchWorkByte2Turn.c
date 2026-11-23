#include "bof3/bof3.h"

typedef void (*BmagicMagic05703Handler)(void);

typedef struct BmagicMagic05703DispatchTable {
  BmagicMagic05703Handler handlers[3];
} BmagicMagic05703DispatchTable;

/*
 * Code pointer table at 0x801EEC38, the three-entry handler table this
 * dispatcher copies: entry 0 = 0x801EF5BC, entry 1 = 0x801EF620, entry 2 =
 * 0x801EF81C. It is the third word group of the state-1 table run whose first
 * group Splat binds at 0x801EEC20, and this target generated
 * undefined_syms_auto.txt already names the word the scaled index is added to,
 * so the symbol is declared here at its own address with no target-map row.
 */
extern BmagicMagic05703DispatchTable D_801EEC38; /* @source 0x801EEC38 @kind data */

/* @source 0x801EF55C
 * @behavior State-1 handler dispatcher of the overlay work sequence, entry 1 of
 *           the three-entry code pointer table at 0x801EEC20 (0x801EF08C,
 *           0x801EF55C, 0x801EF83C) that the entry dispatcher
 *           dispatchWorkByte1Shift indexes with the
 *           scratchpad work object unsigned byte 0x01, so no jal in the image
 *           names this span. It reads the work-object pointer cell at
 *           0x1F800044 once, before it reserves its frame, then materialises the
 *           table base 0x801EEC38 once and copies the three words there into
 *           its own 0x28-byte frame at 0x10, 0x14 and 0x18 as three bare load
 *           and store pairs. Last it reads work byte 0x02 through the pointer it
 *           already holds, scales it by four, adds the frame pointer and invokes
 *           the copied entry with no arguments; the index is not bounds-checked
 *           and the entry it selects on byte value 2 is the handler at
 *           0x801EF81C. It reads no other state, writes none, takes no arguments
 *           and returns nothing through the jr $ra at 0x801EF5B4 inside the
 *           0x28-byte frame that keeps $ra at 0x20.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Turn(void) {
  BmagicMagic05703DispatchTable handlers;

  handlers = D_801EEC38;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
