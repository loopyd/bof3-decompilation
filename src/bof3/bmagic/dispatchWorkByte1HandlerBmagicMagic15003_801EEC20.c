#include "bof3/bof3.h"

typedef void (*BmagicMagic15003Handler)(void);

typedef struct BmagicMagic15003HandlerTable {
  BmagicMagic15003Handler handlers[7];
} BmagicMagic15003HandlerTable;

/* Scratchpad work-object pointer cell 0x1F800044, bound as g_battle_work by
 * this target's own map row and spelled the same way by this target's existing
 * exact lifts. The original materializes the cell base once here and reads the
 * work-object state byte through the loaded pointer after the table copy. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Code pointer table at 0x801EEC04, the seven-entry run this dispatcher copies:
 * 0x801EECA4, 0x801EED60, 0x801EEDCC, 0x801EEE24, 0x801EEEC4, 0x801EEF80 and
 * 0x801EF00C, the code blocks that follow this entry point in this payload. The
 * run sits in the overlay leading data block that this target Splat lists as
 * tabledata, and this target's generated undefined-symbol table already binds
 * it, so no map row is added here. */
extern BmagicMagic15003HandlerTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC20
 * @behavior Dispatches one frame of this magic work handler from the scratchpad
 *           work object. The work-object pointer cell at 0x1F800044 is read
 *           first, ahead of the frame adjustment, and its state byte at offset 1
 *           is read after the table copy as the step index. The seven code
 *           pointer words of the overlay handler table at 0x801EEC04 (0x801EECA4,
 *           0x801EED60, 0x801EEDCC, 0x801EEE24, 0x801EEEC4, 0x801EEF80,
 *           0x801EF00C) are copied into its own 0x38-byte frame at $sp+0x10,
 *           the selected word is read back from that copy, and it is
 *           invoked with no arguments; the index is not bounds-checked. The
 *           epilogue restores $ra from $sp+0x30 and $sp from +0x38 and leaves the
 *           handler answer in $v0, so this entry returns whatever the selected
 *           handler returns. The body writes no cell, has no loop, and no word of
 *           this payload holds this entry address, so it is an overlay entry
 *           reached from outside the payload and its prologue and return path
 *           alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic15003_801EEC20(void) {
  BmagicMagic15003HandlerTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
}
