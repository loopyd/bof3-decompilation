#include "bof3/bof3.h"

typedef void (*Boss00816Handler)(void);

/*
 * Handler table at 0x800C26FC, the table this dispatcher selects from:
 * entry 0 = 0x800C1DAC, entry 1 = 0x800C1DB4, entry 2 = 0x800C1DBC,
 * entry 3 = 0x800C1E08, then further overlay and main-program handlers.
 */
extern Boss00816Handler D_800C26FC[]; /* @source 0x800C26FC @kind data */

/* @source 0x800C1D74
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and tail-calls the handler selected from the
 * 0x800C26FC handler table, so the selected handler's result becomes this
 * function's result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00816_800C1D74(u8 index) {
  D_800C26FC[index]();
}
