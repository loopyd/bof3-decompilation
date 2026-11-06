#include "bof3/bof3.h"

typedef void (*Boss01416Handler)(void);

/*
 * Handler table at 0x800C1BB0, the table this dispatcher selects from: its
 * three words are 0x800C19D0, 0x800C19D8 and 0x800C19E0, the inert handlers of
 * this overlay. They are the last words of the pointer run the payload keeps
 * from 0x800C1B34 to 0x800C1BBC inside the Splat asm span at 0x800C1B28, which
 * disassembles as a word run. The generated undefined_syms_auto.txt row for
 * D_800C1BB0 already binds the word the scaled index is added to, so this
 * declaration needs no map row.
 */
extern Boss01416Handler D_800C1BB0[]; /* @source 0x800C1BB0 @kind data */

/* @source 0x800C1998
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1BB0
 * handler table, so the selected handler's result stays in the return
 * register. The overlay entry point at 0x800C1934 publishes this function's
 * address into the slot at offset 0xE4 of the record it reaches through
 * D_801EB4E8. Same C shape as the exact indexed dispatchers at 0x800C19C8 of
 * BIN/BOSS/BOSS002.EMI#16 and 0x800C198C of BIN/BOSS/BOSS021.EMI#16; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01416_800C1998(u8 index) {
  D_800C1BB0[index]();
}
