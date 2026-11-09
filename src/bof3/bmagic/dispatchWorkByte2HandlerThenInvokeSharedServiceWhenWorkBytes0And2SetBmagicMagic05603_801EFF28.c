#include "bof3/bof3.h"

typedef void (*BmagicMagic05603Byte2Handler)(void);

/* Nine-entry work byte 2 step table of this overlay: the code pointer run at
 * 0x801EEC10 holds 0x801EFFE8, 0x801F0024, 0x801F00C8, 0x801F0178, 0x801F023C,
 * 0x801F042C, 0x801F04EC, 0x801F0658 and 0x801F0724. It follows the three
 * words at 0x801EEC04 (0x801EEC94, 0x801EEF3C, 0x801EF17C) in the overlay's
 * leading data block, which no map row owns, so the reference binds through the
 * target's generated undefined-symbol table. */
typedef struct BmagicMagic05603Byte2DispatchTable {
  BmagicMagic05603Byte2Handler handlers[9];
} BmagicMagic05603Byte2DispatchTable;

extern BmagicMagic05603Byte2DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* Scratchpad work-object pointer cell. The original materialises the cell for
 * the dispatch index and again after the indirect call, so it is read through
 * the target's named symbol, whose address folds into each load */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EFF28
 * @behavior Step dispatcher of the overlay work object, run as entry 3 of the
 *           code pointer run at 0x801F0744. It copies the nine-word work byte 2
 *           handler table at 0x801EEC10 into its own 0x40-byte frame with one
 *           four-word copy loop and a trailing word, so the call target is read
 *           back from the copy, then reads the scratchpad work-object pointer
 *           cell at 0x1F800044 and invokes the copied entry selected by that
 *           object's unsigned byte 2 with no arguments and no bounds test. After
 *           the handler returns it re-reads the cell and, when the work object's
 *           byte 0 and byte 2 are both non-zero, invokes the shared
 *           main-executable service func_8014D290. The nine copied entries are
 *           the step handlers 0x801EFFE8, 0x801F0024, 0x801F00C8, 0x801F0178,
 *           0x801F023C, 0x801F042C, 0x801F04EC, 0x801F0658 and 0x801F0724. No
 *           jal in this payload names 0x801EFF28, so it is reached through that
 *           run alone. It takes no arguments and returns nothing; the frame
 *           keeps the table at 0x10 and $ra at 0x38.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic05603_801EFF28(
    void) {
  BmagicMagic05603Byte2DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC10;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
}
