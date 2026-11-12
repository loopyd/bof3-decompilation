#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800WorkStateTable801EECC8 {
  BmagicMagic00800Handler handlers[10];
} BmagicMagic00800WorkStateTable801EECC8;

/*
 * Code pointer run at 0x801EECC8, the ten-word work-state table this
 * dispatcher copies: entry 0 = 0x801F17BC, entry 1 = 0x801F1804,
 * entry 2 = 0x801F1880, entry 3 = 0x801F18F4, entry 4 = 0x801F1804,
 * entry 5 = 0x801F1880, entry 6 = 0x801F18F4, entry 7 = 0x801F1804,
 * entry 8 = 0x801F1880, entry 9 = 0x801F1BCC. The ten words are the whole
 * overlay-header pointer run: they end at 0x801EECEF and the first code word
 * starts at 0x801EECF0.
 */
extern BmagicMagic00800WorkStateTable801EECC8 D_801EECC8; /* @source 0x801EECC8 @kind data */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * for the dispatch index and again after the indirect call, so it is read
 * through the named target symbol instead of the constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header; this caller passes no
 * arguments. */
void func_8014D290(void);

/* @source 0x801F1708
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its byte at offset 2 selects an entry of the ten-word work-state table at
 * 0x801EECC8 (0x801F17BC, 0x801F1804, 0x801F1880, 0x801F18F4, 0x801F1804,
 * 0x801F1880, 0x801F18F4, 0x801F1804, 0x801F1880, 0x801F1BCC), and that entry
 * is invoked with no arguments. As at 0x801EECF0, 0x801EF384, 0x801EF75C,
 * 0x801F0250, 0x801F0294, 0x801F0568 and 0x801F0964, the table words are
 * copied into this dispatcher's own 0x40-byte frame first, so the call target
 * is read back from the copy; at ten words that copy is a four-word loop over
 * the leading 0x20 bytes plus a two-word tail. After the handler returns it
 * re-reads the pointer cell and, when the work object's flags byte 0 is
 * non-zero, runs the shared main-executable service func_8014D290. Its address
 * is the last word (index 5) of the overlay handler table at 0x801F1BEC that
 * 0x801F0250 dispatches. It returns nothing; the frame keeps $ra at 0x38.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic00800_801F1708(
    void) {
  BmagicMagic00800WorkStateTable801EECC8 handlers;

  handlers = D_801EECC8;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0u) {
    func_8014D290();
  }
}
