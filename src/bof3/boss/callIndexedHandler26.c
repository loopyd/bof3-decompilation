#include "bof3/bof3.h"

typedef void (*Boss02416Handler)(void);

/*
 * Handler table at 0x800C1BC4, the table this dispatcher selects from: its
 * three words are 0x800C19D0, 0x800C19D8 and 0x800C19E0, the inert handlers of
 * this overlay. They are the last three words of the pointer run the payload
 * keeps inside the Splat textbin span at 0x800C1B44, which runs to the end of
 * the payload. The generated undefined_syms_auto.txt row for D_800C1BC4 already
 * binds the word the scaled index is added to, so this declaration needs no map
 * row.
 */
extern Boss02416Handler D_800C1BC4[]; /* @source 0x800C1BC4 @kind data */

/* @source 0x800C1998
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1BC4
 * handler table, so the selected handler's result stays in the return
 * register. The overlay entry point at 0x800C1934 publishes this function's
 * address into the slot at offset 0xE4 of the record it reaches through
 * D_801EB4E8. Same C shape as the exact indexed dispatcher at 0x800C18A0 of
 * this overlay and at 0x800C1998 of BIN/BOSS/BOSS014.EMI#16; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler26(u8 index) {
  D_800C1BC4[index]();
}
