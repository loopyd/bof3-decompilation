#include "bof3/bof3.h"

typedef void (*Boss02816Handler)(void);

/*
 * Handler table at 0x800C2198, the table this dispatcher selects from:
 * entry 0 = 0x800C1B38 (inert handler), entry 1 = 0x800C1C30,
 * entry 2 = 0x800C1FB8, entry 3 = 0x800C1C9C, entry 4 = 0x800C1D90,
 * entry 5 = 0x800C1E80, entry 6 = 0x800C1F2C, entry 7 = 0x800C2024,
 * entry 8 = 0x800C20C8, entry 9 = 0x800C2120. The base sits inside the
 * overlay's pointer run labelled T_800C2180, which binds as one reviewed data
 * row.
 */
extern Boss02816Handler D_800C2198[]; /* @source 0x800C2198 @kind data */

/* @source 0x800C1AF0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2198
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through the overlay entry point. Same C
 * shape as the sibling indexed dispatchers at 0x800C19C8 of boss002/16,
 * 0x800C1C88 of boss008/16 and 0x800C1B18 of boss018/16; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02816_800C1AF0(u8 index) {
  D_800C2198[index]();
}
