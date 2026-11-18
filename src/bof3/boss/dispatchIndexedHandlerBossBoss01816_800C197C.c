#include "bof3/bof3.h"

typedef void (*Boss01816Handler)(void);

/*
 * Handler table at 0x800C2CDC, the table this dispatcher selects from:
 * entry 0 = 0x800C19B4, entry 1 = 0x800C19D8 (inert handler), entry 2 =
 * 0x800C19E0 (inert handler), entry 3 = 0x800C1A2C, entry 4 = 0x800C1ADC,
 * then further main-program handlers. The table sits inside the pointer run
 * of this overlay that starts at 0x800C2C38.
 */
extern Boss01816Handler D_800C2CDC[]; /* @source 0x800C2CDC @kind data */

/* @source 0x800C197C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2CDC
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address. Same C shape as the sibling dispatchers at 0x800C1B18,
 * 0x800C1C6C and 0x800C2074 of this overlay; only the table address in the
 * load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01816_800C197C(u8 index) {
  D_800C2CDC[index]();
}
