#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/*
 * Handler table at 0x800C2084, the table this dispatcher selects from:
 * entry 0 = 0x800C1D38, entry 1 = 0x800C1D40, entry 2 = 0x800C1D48, the three
 * inert handlers that follow this function. The base is an interior word of the
 * overlay's pointer run at 0x800C1F9C, which stays one reviewed data row; this
 * word is named at the index the scaled argument adds to.
 */
extern Boss00216Handler D_800C2084[]; /* @source 0x800C2084 @kind data */

/* @source 0x800C1D00
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2084
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's address;
 * the main program reaches it through its overlay entry point. Same shape as
 * the sibling indexed dispatcher at 0x800C19C8 of this overlay; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00216_800C1D00(u8 index) {
  D_800C2084[index]();
}
