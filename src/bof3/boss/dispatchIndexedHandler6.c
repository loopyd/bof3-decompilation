#include "bof3/bof3.h"

typedef void (*Boss02416Handler)(void);

/*
 * Handler table at 0x800C1B88, the table this dispatcher selects from: its
 * four words are 0x800C18D8, 0x800C18E0 and 0x800C18E8, the inert handlers of
 * this overlay, then the overlay entry point 0x800C1934. The base is an
 * interior word of the pointer run the payload keeps inside the Splat
 * textbin span at 0x800C1B44. The generated undefined_syms_auto.txt row for
 * D_800C1B88 already binds the word the scaled index is added to, so this
 * declaration needs no map row.
 */
extern Boss02416Handler D_800C1B88[]; /* @source 0x800C1B88 @kind data */

/* @source 0x800C18A0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1B88
 * handler table, so the selected handler's result stays in the return
 * register. No instruction in this payload references this function's
 * address; the main program reaches it through the overlay entry point at
 * 0x800C1934. Same body as the exact indexed dispatcher at 0x800C18A0 of
 * BIN/BOSS/BOSS014.EMI#16, proven by a cmp of the two 0x38 spans in which the
 * single differing byte is the low byte of this load word's offset immediate
 * (0x88 here, 0x74 there); both payloads keep the same four handler words at
 * their table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandler6(u8 index) {
  D_800C1B88[index]();
}
