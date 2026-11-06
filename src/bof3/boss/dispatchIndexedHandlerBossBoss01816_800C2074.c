#include "bof3/bof3.h"

typedef void (*Boss01816Handler)(void);

/*
 * Handler table at 0x800C2DB8, the table this dispatcher selects from:
 * entry 0 = 0x800C20AC (boss state byte clear), entry 1 = 0x800C20BC,
 * entry 2 = 0x800C210C (inert handler).  The word at 0x800C2DC4 is not an
 * in-image pointer, so the run ends after entry 2.
 */
extern Boss01816Handler D_800C2DB8[]; /* @source 0x800C2DB8 @kind data */

/* @source 0x800C2074
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2DB8
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address. Same C shape as the sibling dispatchers at 0x800C1B18 and
 * 0x800C1C6C of this overlay; only the table address in the load word
 * differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01816_800C2074(u8 index) {
  D_800C2DB8[index]();
}
