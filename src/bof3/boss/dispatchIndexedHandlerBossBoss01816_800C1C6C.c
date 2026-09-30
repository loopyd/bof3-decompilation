#include "bof3/bof3.h"

typedef void (*Boss01816Handler)(void);

/*
 * Handler table at 0x800C2D54, the table this dispatcher selects from:
 * entry 0 = 0x800C1CA4, entry 1 = 0x800C1CC8 (inert handler), entry 2 =
 * 0x800C1CD0 (inert handler), entry 3 = 0x800C1DC4, then further
 * main-program handlers.
 */
extern Boss01816Handler D_800C2D54[]; /* @source 0x800C2D54 @kind data */

/* @source 0x800C1C6C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2D54
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address. Same C shape as the sibling dispatcher at 0x800C1B18 of this
 * overlay; only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01816_800C1C6C(u8 index) {
  D_800C2D54[index]();
}
