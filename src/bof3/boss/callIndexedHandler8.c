#include "bof3/bof3.h"

typedef void (*Boss01416Handler)(void);

/*
 * Handler table at 0x800C1B74, the table this dispatcher selects from: its
 * four words are 0x800C18D8, 0x800C18E0 and 0x800C18E8, the inert handlers of
 * this overlay, then the overlay entry point 0x800C1934. The base is an
 * interior word of the pointer run the payload keeps from 0x800C1B34 to
 * 0x800C1BBC inside the Splat asm span at 0x800C1B28, which disassembles as a
 * word run. The generated undefined_syms_auto.txt row for D_800C1B74 already
 * binds the word the scaled index is added to, so this declaration needs no
 * map row.
 */
extern Boss01416Handler D_800C1B74[]; /* @source 0x800C1B74 @kind data */

/* @source 0x800C18A0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1B74
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through the overlay entry point at
 * 0x800C1934. Same C shape as the exact sibling indexed dispatcher at
 * 0x800C1998 of this overlay; only the table address in the load word
 * differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler8(u8 index) {
  D_800C1B74[index]();
}
