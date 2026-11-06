#include "bof3/bof3.h"

typedef void (*Boss00113Handler)(void);

/*
 * Handler table at 0x800C2270, the table this dispatcher selects from:
 * entry 0 = 0x800C1E38, entry 1 = 0x800C1E40, entry 2 = 0x800C1E48, all
 * inert handlers of this overlay.
 */
extern Boss00113Handler D_800C2270[]; /* @source 0x800C2270 @kind data */

/* @source 0x800C1E00
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2270
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through the overlay entry point. Same
 * C shape as the already-exact dispatchers at 0x800C1E58 of
 * BIN/BOSS/BOSS013.EMI#16 and at 0x800C1B18 of BIN/BOSS/BOSS018.EMI#16; only
 * the table address in the load word differs, which is the two bytes that
 * separate this body from the boss013/16 sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00113_800C1E00(u8 index) {
  D_800C2270[index]();
}
