#include "bof3/bof3.h"

typedef void (*Boss00816Handler)(void);

/*
 * Handler table at 0x800C26C0, the table this dispatcher selects from:
 * entry 0 = 0x800C1CC0, entry 1 = 0x800C1CC8, entry 2 = 0x800C1CD0,
 * entry 3 = 0x800C1D1C, then further overlay and main-program handlers.
 */
extern Boss00816Handler D_800C26C0[]; /* @source 0x800C26C0 @kind data */

/* @source 0x800C1C88
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and tail-calls the handler selected from the
 * 0x800C26C0 handler table, so the selected handler's result becomes this
 * function's result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00816_800C1C88(u8 index) {
  D_800C26C0[index]();
}
