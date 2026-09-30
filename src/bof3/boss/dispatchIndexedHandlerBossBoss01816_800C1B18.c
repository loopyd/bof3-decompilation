#include "bof3/bof3.h"

typedef void (*Boss01816Handler)(void);

/*
 * Handler table at 0x800C2D18, the table this dispatcher selects from:
 * entry 0 = 0x800C1B50, entry 1 = 0x800C1B74 (inert handler), entry 2 =
 * 0x800C1B7C (inert handler), entry 3 = 0x800C1BC8, then further
 * main-program handlers.
 */
extern Boss01816Handler D_800C2D18[]; /* @source 0x800C2D18 @kind data */

/* @source 0x800C1B18
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2D18
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address. Same C shape as the sibling dispatchers at 0x800C1C88 of
 * boss008/16 and at 0x800C1C6C of this overlay; only the table address in
 * the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01816_800C1B18(u8 index) {
  D_800C2D18[index]();
}
