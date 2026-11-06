#include "bof3/bof3.h"

typedef void (*Boss02116Handler)(void);

/*
 * Handler table at 0x800C1D20, the table this dispatcher selects from:
 * entry 0 = 0x800C19C4, entry 1 = 0x800C19D4 and entry 2 = 0x800C19DC (inert
 * handlers of this overlay), entry 3 = 0x800C19E4, then main-program handlers
 * from 0x801E52E8 on. The base sits inside the pointer run bound as the
 * reviewed textbin row at 0x800C1CB0, whose pointer words start at 0x800C1CD8.
 * The dispatchers at 0x800C1804 and 0x800C18B8 index the same run from its words
 * 0 and 12, so the run stays one row and this declaration binds only word 18,
 * the word the scaled index is added to.
 */
extern Boss02116Handler D_800C1D20[]; /* @source 0x800C1D20 @kind data */

/* @source 0x800C198C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1D20
 * handler table, so the selected handler's result stays in the return
 * register. The overlay entry point at 0x800C1860 publishes this function's
 * address into the slot at offset 0xE4 of the record it reaches through
 * D_801EB4E8. Same C shape as the exact indexed dispatchers at 0x800C18C4 of
 * BIN/BOSS/BOSS035.EMI#16 and 0x800C19C8 of BIN/BOSS/BOSS002.EMI#16; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02116_800C198C(u8 index) {
  D_800C1D20[index]();
}
