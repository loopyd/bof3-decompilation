#include "bof3/bof3.h"

typedef void (*Boss01516Handler)(void);

/*
 * Handler table at 0x800C19E0, the table this dispatcher selects from: its
 * three words are 0x800C18D8, 0x800C18E0 and 0x800C18E8, the inert handlers of
 * this overlay. They are the last words of the pointer run the payload keeps
 * from 0x800C19BC to its end at 0x800C19EC, reviewed as the textbin row at
 * 0x800C199C, so this declaration binds only the word the scaled index is
 * added to and the run stays one row.
 */
extern Boss01516Handler D_800C19E0[]; /* @source 0x800C19E0 @kind data */

/* @source 0x800C18A0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C19E0
 * handler table, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled table
 * load, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. No word of this payload names this function's address and no jal
 * targets it, so it is an overlay entry reached from outside the payload.
 * Same C shape as the exact indexed dispatcher at 0x800C18A0 of
 * BIN/BOSS/BOSS014.EMI#16; a payload comparison of the two 0x38-byte spans
 * found two differing bytes, both inside the table load word's immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01516_800C18A0(u8 index) {
  D_800C19E0[index]();
}
