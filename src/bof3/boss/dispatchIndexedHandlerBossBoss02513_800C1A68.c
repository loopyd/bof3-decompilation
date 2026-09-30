#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/*
 * Handler table at 0x800C31E4, the table this dispatcher selects from:
 * entry 0 = 0x800C1AA0, entry 1 = 0x800C1AF8 (inert handler), entry 2 =
 * 0x800C1B00.
 */
extern Boss02513Handler D_800C31E4[]; /* @source 0x800C31E4 @kind table */

/* @source 0x800C1A68
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C31E4
 * handler table, so the selected handler's result stays in the return
 * register. No in-payload caller: func_800C1894 materialises this function's
 * address instead of branching to it. Byte-identical to the sibling
 * dispatcher at 0x800C1D2C of this overlay; only the table address in the
 * load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02513_800C1A68(u8 index) {
  D_800C31E4[index]();
}
