#include "bof3/bof3.h"

typedef void (*Boss01316Handler)(void);

/*
 * Handler table at 0x800C26D4, the table this dispatcher selects from:
 * entry 0 = 0x800C1E90, entry 1 = 0x800C1F24, entry 2 = 0x800C2004 (inert
 * handler), entry 3 = 0x00000000.
 */
extern Boss01316Handler D_800C26D4[]; /* @source 0x800C26D4 @kind data */

/* @source 0x800C1E58
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C26D4
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through the overlay entry point. Same
 * C shape as the already-exact dispatchers at 0x800C18FC and 0x800C1A04 of
 * this overlay and at 0x800C1D74 of BIN/BOSS/BOSS008.EMI#16; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01316_800C1E58(u8 index) {
  D_800C26D4[index]();
}
