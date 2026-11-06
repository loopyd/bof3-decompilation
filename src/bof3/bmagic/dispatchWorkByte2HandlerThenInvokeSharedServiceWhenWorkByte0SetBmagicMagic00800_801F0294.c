#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800WorkStateTable {
  BmagicMagic00800Handler handlers[6];
} BmagicMagic00800WorkStateTable;

/*
 * Code pointer table at 0x801EEC58, the six-entry work-state table this
 * dispatcher copies: entry 0 = 0x801F0330, entry 1 = 0x801F037C,
 * entry 2 = 0x801F03FC, entry 3 = 0x801F047C, entry 4 = 0x801F04DC,
 * entry 5 = 0x801F0548.
 */
extern BmagicMagic00800WorkStateTable D_801EEC58; /* @source 0x801EEC58 @kind data */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * for the dispatch index and again after the indirect call, so it is read
 * through the named target symbol instead of the constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801F0294
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its byte at offset 2 selects an entry of the six-entry work-state table at
 * 0x801EEC58 (0x801F0330, 0x801F037C, 0x801F03FC, 0x801F047C, 0x801F04DC,
 * 0x801F0548), and that entry is invoked with no arguments. As at 0x801EECF0,
 * 0x801EF384, 0x801EF75C, 0x801F0568 and 0x801F0964, the table words are copied
 * into this dispatcher's own 0x30-byte frame first, so the call target is read
 * back from the copy. After the handler returns it re-reads the pointer cell
 * and, when the work object's flags byte 0 is non-zero, runs the shared
 * main-executable service func_8014D290. Its address is the first word (index
 * 0) of the six-entry work byte 1 table at 0x801F1BEC that 0x801F0250
 * dispatches. It returns nothing; the frame keeps $ra at 0x28.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic00800_801F0294(
    void) {
  BmagicMagic00800WorkStateTable handlers;

  handlers = D_801EEC58;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0u) {
    func_8014D290();
  }
}
