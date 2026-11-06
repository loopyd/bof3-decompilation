#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/*
 * Handler table at 0x800C1FF4, the table this dispatcher selects from:
 * entry 0 = 0x800C1A00, entry 1 = 0x800C1A4C, entry 2 = 0x800C1AA0 (inert
 * handler), entry 3 = 0x800C1AEC, then further overlay and main-program
 * handlers. The base sits inside the overlay's pointer run at 0x800C1F9C, which
 * binds as one reviewed data row.
 */
extern Boss00216Handler D_800C1FF4[]; /* @source 0x800C1FF4 @kind data */

/* @source 0x800C19C8
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1FF4
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through its overlay entry point. Same C
 * shape as the sibling indexed dispatchers at 0x800C1C88 of boss008/16 and
 * 0x800C1B18 of boss018/16; only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00216_800C19C8(u8 index) {
  D_800C1FF4[index]();
}
