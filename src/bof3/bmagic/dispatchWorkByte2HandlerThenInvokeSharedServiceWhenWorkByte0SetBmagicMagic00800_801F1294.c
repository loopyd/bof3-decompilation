#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800WorkStateTable801EECAC {
  BmagicMagic00800Handler handlers[7];
} BmagicMagic00800WorkStateTable801EECAC;

/*
 * Code pointer table at 0x801EECAC, the seven-entry handler run this
 * dispatcher copies: entry 0 = 0x801F133C, entry 1 = 0x801F1384,
 * entry 2 = 0x801F1400, entry 3 = 0x801F1474, entry 4 = 0x801F1384,
 * entry 5 = 0x801F1400, entry 6 = 0x801F16E8. It sits in the overlay leading
 * data block this target Splat lists as tabledata and the target's generated
 * undefined-symbol table already binds it, so no map row is added here.
 */
extern BmagicMagic00800WorkStateTable801EECAC
    D_801EECAC; /* @source 0x801EECAC @kind data */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * for the dispatch index and again after the indirect call, so it is read
 * through the named target symbol instead of the constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header; this caller passes no
 * arguments. */
void func_8014D290(void);

/* @source 0x801F1294
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its byte at offset 2 selects an entry of the seven-entry handler run at
 * 0x801EECAC (0x801F133C, 0x801F1384, 0x801F1400, 0x801F1474, 0x801F1384,
 * 0x801F1400, 0x801F16E8), and that entry is invoked with no arguments. As at
 * 0x801EECF0, 0x801EF384, 0x801EF75C, 0x801F0250, 0x801F0294, 0x801F0568 and
 * 0x801F0964, the table words are copied into this dispatcher's own 0x38-byte
 * frame first, so the call target is read back from the copy. After the
 * handler returns it re-reads the pointer cell and, when the work object's
 * flags byte 0 is non-zero, runs the shared main-executable service
 * func_8014D290. Its address is index 4 of the six-entry overlay handler table
 * at 0x801F1BEC that 0x801F0250 dispatches. It returns nothing; the frame keeps
 * $ra at 0x30.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic00800_801F1294(
    void) {
  BmagicMagic00800WorkStateTable801EECAC handlers;

  handlers = D_801EECAC;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0u) {
    func_8014D290();
  }
}
