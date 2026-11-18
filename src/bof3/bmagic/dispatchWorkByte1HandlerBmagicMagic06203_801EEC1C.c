#include "bof3/bof3.h"

typedef void (*BmagicMagic06203Handler)(void);

typedef struct BmagicMagic06203DispatchTable {
  BmagicMagic06203Handler handlers[6];
} BmagicMagic06203DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the six-entry table this dispatcher
 * copies: entry 0 = 0x801EEC94, entry 1 = 0x801EEEC0, entry 2 = 0x801EEF68,
 * entry 3 = 0x801EF078, entry 4 = 0x801EF0C0, entry 5 = 0x801EF2A0, the six
 * code blocks that follow this overlay entry point and fill the leading data
 * block from 0x801EEC04 up to the first code address 0x801EEC1C. The run sits
 * in that leading data block, which no map row owns, so the reference is
 * bound by the target generated undefined-symbol table.
 *
 * The 0x78-byte body is byte-identical to the six-entry dispatchers at
 * 0x801EEC1C of emi/bmagic/magic086/03 and at 0x801EEC38 of
 * emi/bmagic/magic041/03. One scan of the 398 extracted payloads under
 * out/binaries compared this span word for word and found it unchanged in
 * emi/bmagic/magic020/03, magic041/03, magic082/03, magic086/03,
 * magic110/03 and magic167/03 as well. The clone adapter refused that pair on
 * the shared dispatcher macro the sibling body reaches, so this source is
 * transcribed from the exact lift at emi/bmagic/magic086/03 with this
 * target own entry addresses.
 */
extern BmagicMagic06203DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC1C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 1 selects an entry of the
 * six-entry code pointer table at 0x801EEC04 (0x801EEC94, 0x801EEEC0,
 * 0x801EEF68, 0x801EF078, 0x801EF0C0, 0x801EF2A0), and that entry is invoked
 * with no arguments. The six table words are copied into this function own
 * 0x30-byte frame first, so the call target is read back from the copy and the
 * selector is not bounds-checked. It reads no other state and writes none.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic06203_801EEC1C(void) {
  BmagicMagic06203DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
