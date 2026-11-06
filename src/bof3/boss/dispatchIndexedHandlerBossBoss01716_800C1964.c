#include "bof3/bof3.h"

typedef void (*Boss01716Handler)(void);

/*
 * Handler table at 0x800C1AF4, the table this dispatcher selects from: its
 * three words are 0x800C199C, 0x800C19A4 and 0x800C19AC, the inert handlers of
 * this overlay, and the word after them is zero. The base sits inside the
 * pointer run bound as the reviewed textbin row at 0x800C1A70, whose pointer
 * words start at 0x800C1AAC; the dispatchers at 0x800C1804 and 0x800C18A0
 * index that same run from its words 0 and 12, so the run stays one row and
 * this declaration binds only word 18, the word the scaled index is added to.
 * The generated undefined_syms_auto.txt row for D_800C1AF4 already binds it,
 * so this declaration needs no map row.
 */
extern Boss01716Handler D_800C1AF4[]; /* @source 0x800C1AF4 @kind data */

/* @source 0x800C1964
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1AF4
 * handler table, so the selected handler's result stays in the return
 * register. All three table words are inert handlers of this overlay, so an
 * index in range returns without touching any state. No instruction in this
 * payload references this function's address; the main program reaches it
 * through its overlay entry point. Same C shape as the exact indexed
 * dispatchers at 0x800C1978 of BIN/BOSS/BOSS032.EMI#16 and at 0x800C198C of
 * BIN/BOSS/BOSS021.EMI#16; only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01716_800C1964(u8 index) {
  D_800C1AF4[index]();
}
