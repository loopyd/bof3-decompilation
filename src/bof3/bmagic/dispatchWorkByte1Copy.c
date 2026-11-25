#include "bof3/bof3.h"

typedef void (*BmagicMagic11003Handler)(void);

typedef struct BmagicMagic11003DispatchTable {
  BmagicMagic11003Handler handlers[6];
} BmagicMagic11003DispatchTable;

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled the same way by this target's
 * existing exact lifts. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Code pointer table at 0x801EEC04, the six-entry run this dispatcher copies:
 * entry 0 = 0x801EEC94, entry 1 = 0x801EEE68, entry 2 = 0x801EEEDC,
 * entry 3 = 0x801EEF80, entry 4 = 0x801EF068, entry 5 = 0x801EF114, the six
 * code blocks that follow this overlay's entry point in this payload. The run
 * sits in the overlay's leading data block, which no map row owns, so the
 * reference is bound by this target's generated undefined-symbol table. */
extern BmagicMagic11003DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC1C
 * @behavior Dispatches one frame of the overlay work handler from the
 *           scratchpad work object: the work-object pointer cell at 0x1F800044
 *           is read, its state byte at offset 1 selects an entry of the
 *           six-entry code pointer table at 0x801EEC04 (0x801EEC94, 0x801EEE68,
 *           0x801EEEDC, 0x801EEF80, 0x801EF068, 0x801EF114), and that entry is
 *           invoked with no arguments. The six table words are copied into this
 *           function's own 0x30-byte frame first, so the call target is read
 *           back from the copy; the cell load leads the body, ahead of the frame
 *           adjustment, and the index is not bounds-checked. The selected
 *           handler's $v0 is left in place by the epilogue, so this entry
 *           returns whatever that handler returns. It reads no other state and
 *           writes none, and no word of this payload holds its own address, so
 *           it is an overlay entry reached from outside the payload; its
 *           prologue and return path alone place the boundary.
 *           The span 0x801EEC1C..0x801EEC93 is byte-identical to
 *           0x801EEC1C..0x801EEC93 of emi/bmagic/magic086/03, whose exact lift
 *           dispatchWorkByte1HandlerBmagicMagic08603_801EEC1C is the
 *           representative this body is seeded from; its statement order,
 *           operand forms and declaration style are kept, and the scratchpad
 *           cell keeps this target's mapped spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Copy(void) {
  BmagicMagic11003DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
}
