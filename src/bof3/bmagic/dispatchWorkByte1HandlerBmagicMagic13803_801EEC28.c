#include "bof3/bof3.h"

typedef void (*BmagicMagic13803Handler)(void);

typedef struct BmagicMagic13803DispatchTable {
  BmagicMagic13803Handler handlers[3];
} BmagicMagic13803DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the leading three words this dispatcher
 * copies: entry 0 = 0x801EEC88, entry 1 = 0x801EED60, entry 2 = 0x801EEED8.
 * The run continues past these three words for nine entries total, but this
 * body copies only its prefix. The run sits in the overlay's leading data
 * block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 *
 * The 0x60-byte body is byte-identical to the three-entry dispatchers at
 * 0x801EEC34 of emi/bmagic/magic039/03 and 0x801EEC64 of emi/bmagic/magic001/00,
 * proved by comparing this payload's span with theirs: one scan of the
 * extracted payloads found 26 overlays carrying the same 0x60 bytes.
 */
extern BmagicMagic13803DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC28
 * @behavior Dispatches one frame of the overlay's leading work handler from
 *           the scratchpad work object: the work-object pointer cell at
 *           0x1F800044 is read, its state byte at offset 1 selects an entry of
 *           the three-entry code pointer prefix at 0x801EEC04 (0x801EEC88,
 *           0x801EED60, 0x801EEED8), and that entry is invoked with no
 *           arguments. The three table words are copied into this function's
 *           own 0x28-byte frame first, so the call target is read back from
 *           the copy. It reads no other state and writes none, and no word of
 *           this payload holds its own address, so it is an overlay entry
 *           reached from outside the payload; its prologue and return path
 *           alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic13803_801EEC28(void) {
  BmagicMagic13803DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
