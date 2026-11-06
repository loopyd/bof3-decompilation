#include "bof3/bof3.h"

typedef void (*Boss03516Handler)(void);

/*
 * Handler table at 0x800C1D5C, the table this dispatcher selects from:
 * entry 0 = 0x800C18FC and entry 1 = 0x800C1904 (inert handlers of this
 * overlay), entry 2 = 0x800C190C, entry 3 = 0x800C1994, then main-program and
 * companion-overlay handlers from 0x801E52E8 on. The base sits inside the
 * pointer run bound as the reviewed textbin row at 0x800C1D04, and the
 * dispatcher at 0x800C1950 indexes the same run from its fourth word, so the
 * run stays one row and this declaration binds only the word the scaled index
 * is added to.
 */
extern Boss03516Handler D_800C1D5C[]; /* @source 0x800C1D5C @kind data */

/* @source 0x800C18C4
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1D5C
 * handler table, so the selected handler's result stays in the return
 * register. The overlay entry point at 0x800C1860 publishes this function's
 * address into the slot at offset 0xE4 of the record it reaches through
 * D_801EB4E8. Same C shape as the exact indexed dispatchers at 0x800C1D60 of
 * BIN/BOSS/BOSS034.EMI#16 and 0x800C19C8 of BIN/BOSS/BOSS002.EMI#16; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03516_800C18C4(u8 index) {
  D_800C1D5C[index]();
}
