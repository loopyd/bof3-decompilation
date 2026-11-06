#include "bof3/bof3.h"

typedef void (*Boss05016Handler)(void);

/*
 * Handler table at 0x800C1B7C, the table this dispatcher selects from: its
 * three words are 0x800C18F0, 0x800C18F8 and 0x800C1900, the inert handlers of
 * this overlay. They are the last words of the pointer run the payload keeps
 * from 0x800C1B34 to its end at 0x800C1B88, reviewed as the textbin row at
 * 0x800C1B34, so this declaration binds only the word the scaled index is
 * added to and the run stays one row.
 */
extern Boss05016Handler D_800C1B7C[]; /* @source 0x800C1B7C @kind data */

/* @source 0x800C18B8
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1B7C
 * handler table, so the selected handler's result stays in the return
 * register. The overlay entry point at 0x800C1854 publishes this function's
 * address into the slot at offset 0xE4 of the record it reaches through
 * 0x801FB4E8. Same C shape as the exact indexed dispatchers at 0x800C18C4 of
 * BIN/BOSS/BOSS035.EMI#16 and 0x800C1998 of BIN/BOSS/BOSS014.EMI#16; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss05016_800C18B8(u8 index) {
  D_800C1B7C[index]();
}
