#include "bof3/bof3.h"

typedef void (*Boss03516Handler)(void);

/*
 * Handler table at 0x800C1DD4, the table this dispatcher selects from:
 * entry 0 = 0x800C1B34, entry 1 = 0x800C1B3C and entry 2 = 0x800C1B44, the
 * three inert handlers that follow this function, so every index the caller
 * passes returns straight to it. The base sits inside the pointer run bound as
 * the reviewed textbin row at 0x800C1D04, which stays one row and this
 * declaration binds only the word the scaled index is added to.
 */
extern Boss03516Handler D_800C1DD4[]; /* @source 0x800C1DD4 @kind data */

/* @source 0x800C1AFC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1DD4
 * handler table, so the selected handler's result stays in the return
 * register. All three entries are the inert handlers 0x800C1B34, 0x800C1B3C
 * and 0x800C1B44 that follow this function. The overlay entry point at
 * 0x800C1A98 publishes this function's address into the slot at offset 0xE4 of
 * the record it reaches through D_801EB4E8, next to the record's other handler
 * bases 0x800C1D04 at 0xEC and 0x800C1D1C at 0xE8. Same C shape as the sibling
 * indexed dispatcher at 0x800C18C4 of this overlay and the exact dispatcher at
 * 0x800C1D00 of BIN/BOSS/BOSS002.EMI#16; only the table address in the load
 * word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03516_800C1AFC(u8 index) {
  D_800C1DD4[index]();
}
