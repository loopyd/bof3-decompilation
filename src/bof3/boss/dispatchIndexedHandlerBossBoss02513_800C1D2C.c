#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/*
 * Handler table at 0x800C3238, the table this dispatcher selects from:
 * entry 0 = 0x800C1D64, entry 1 = 0x800C1D78 (inert handler), entry 2 =
 * 0x800C1D80 (inert handler).
 */
extern Boss02513Handler D_800C3238[]; /* @source 0x800C3238 @kind table */

/* @source 0x800C1D2C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C3238
 * handler table, so the selected handler's result stays in the return
 * register. No in-payload caller: func_800C1BB0 materialises this function's
 * address instead of branching to it. Byte-identical to the sibling
 * dispatcher at 0x800C1A68 of this overlay; only the table address in the
 * load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02513_800C1D2C(u8 index) {
  D_800C3238[index]();
}
