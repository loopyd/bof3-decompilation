#include "bof3/bof3.h"

typedef void (*Boss03416Handler)(void);

/*
 * Handler table at 0x800C2730, the table this dispatcher selects from:
 * entry 0 = 0x800C1D98, entry 1 = 0x800C1DA0, entry 2 = 0x800C1DA8,
 * entry 3 = 0x800C1DF4, then further overlay and main-program handlers. The
 * base sits inside the overlay's pointer run at 0x800C26B8, which binds as one
 * reviewed data row.
 */
extern Boss03416Handler D_800C2730[]; /* @source 0x800C2730 @kind data */

/* @source 0x800C1D60
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2730
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through its overlay entry point. Same C
 * shape as the sibling indexed dispatchers at 0x800C1C88 of boss008/16 and
 * 0x800C19C8 of boss002/16; only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03416_800C1D60(u8 index) {
  D_800C2730[index]();
}
