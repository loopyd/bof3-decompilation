#include "bof3/bof3.h"

typedef void (*BmagicMagic00903Handler)(void);

typedef struct BmagicMagic00903DispatchTable {
  BmagicMagic00903Handler handlers[5];
} BmagicMagic00903DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC98, entry 1 = 0x801EEF08, entry 2 = 0x801EF01C, entry 3 = 0x801EF068
 * and entry 4 = 0x801EF09C, the five code blocks that follow the overlay's
 * entry point. The run continues at 0x801EEC18 with the four words 0x801EF740,
 * 0x801EF788, 0x801EF814 and 0x801EF87C, which form the table the dispatcher at
 * 0x801EF6B0 copies into its own frame, so this declaration covers only the
 * prefix this function copies. The run sits in the overlay's leading data
 * block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 */
extern BmagicMagic00903DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC28
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read before the frame is created, its unsigned work byte at
 *           offset 1 selects an entry of the five-entry code pointer run at
 *           0x801EEC04 (0x801EEC98, 0x801EEF08, 0x801EF01C, 0x801EF068,
 *           0x801EF09C), and that entry is invoked with no arguments. The five
 *           table words are copied into this function's own 0x30-byte frame
 *           first, three loads and three stores followed by two and two, so the
 *           call target is read back from the copy at sp+0x10+index*4 and the
 *           index is not bounds-checked. It reads no other state, writes none
 *           and returns nothing, and no word of this payload holds its own
 *           address, so the prologue and the return path alone place the
 *           boundary. The 0x70-byte body measured word for word equal to the
 *           dispatchers at 0x801EEC28 of emi/bmagic/magic004/03, at 0x801EEC24
 *           of emi/bmagic/magic067/03 and at 0x801EEC18 of emi/bmagic/magic099/03,
 *           each lifted exact in its own target. This target reaches work byte
 *           1 a second time from 0x801EF0DC, which indexes the tail run at
 *           0x801EF89C in place instead of copying it, so the copy of the
 *           leading run is what this span names.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Nine(void) {
  BmagicMagic00903DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
