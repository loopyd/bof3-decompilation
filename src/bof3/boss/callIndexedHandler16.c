#include "bof3/bof3.h"

typedef void (*Boss02316Handler)(void);

/*
 * Handler table at 0x800C1AA0, the table this dispatcher selects from: entry 0 =
 * 0x800C196C, entry 1 = 0x800C1974 and entry 2 = 0x800C197C, the three inert
 * handlers of this overlay. The base sits inside the pointer run bound as the
 * reviewed textbin row at 0x800C1A40, whose last three words are those entries,
 * so the run stays one row and this declaration binds only the word the scaled
 * index is added to.
 */
extern Boss02316Handler D_800C1AA0[]; /* @source 0x800C1AA0 @kind data */

/* @source 0x800C1934
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 *           scales it by four and calls the handler selected from the 0x800C1AA0
 *           handler table, so the selected handler's result stays in the return
 *           register. The table holds this overlay's three inert handlers, so a
 *           dispatch through it has no side effect. The installer at 0x800C1848
 *           publishes this function's address into the slot at offset 0xE4 of the
 *           record it reaches through D_801EB4E8, the same slot the boss021/16
 *           dispatcher occupies. Same C shape as the exact indexed dispatcher at
 *           0x800C198C of BIN/BOSS/BOSS021.EMI#16, whose 56-byte body differs in
 *           the low half of its table load word alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler16(u8 index) {
  D_800C1AA0[index]();
}
