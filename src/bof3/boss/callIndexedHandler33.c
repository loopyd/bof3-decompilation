#include "bof3/bof3.h"

typedef void (*Boss03416Handler)(void);

/*
 * Handler table at 0x800C276C, the table this dispatcher selects from: entry 0
 * = 0x800C1E90, entry 1 = 0x800C1E98, entry 2 = 0x800C1EA0, entry 3 = 0. The
 * base sits inside the overlay's pointer run at 0x800C26B8, which binds as one
 * reviewed data row.
 */
extern Boss03416Handler D_800C276C[]; /* @source 0x800C276C @kind data */

/* @source 0x800C1E58
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C276C
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through its overlay entry point. Same C
 * shape as the sibling indexed dispatcher at 0x800C1D60 of this overlay, whose
 * only differing word is the table address in the load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler33(u8 index) {
  D_800C276C[index]();
}
