#include "bof3/bof3.h"

typedef void (*Boss00413Handler)(void);

/*
 * Handler table at 0x800C2794, the table this dispatcher selects from:
 * entry 0 = 0x800C1BF4, entry 1 = 0x800C1BFC, entry 2 = 0x800C1C04, the
 * overlay's three inert handlers. The base sits at word 29 of the overlay's
 * pointer run that starts at 0x800C2720 and binds as one reviewed data row,
 * and its three entries are the payload's last twelve bytes.
 */
extern Boss00413Handler D_800C2794[]; /* @source 0x800C2794 @kind data */

/* @source 0x800C1BBC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2794
 * handler table, so the selected handler's result stays in the return
 * register. Every reachable entry of that table is an inert handler, so the
 * overlay's 0x800C1BBC entry point has no observable effect. No word in this
 * payload holds this function's address; the main program reaches it through
 * the overlay entry. Same C shape as the sibling indexed dispatchers at
 * 0x800C1A68 of boss025/13 and 0x800C1C88 of boss008/16; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00413_800C1BBC(u8 index) {
  D_800C2794[index]();
}
