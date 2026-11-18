#include "bof3/bof3.h"

typedef void (*Boss02916Handler)(void);

/*
 * Handler table at 0x800C19EC, the table this dispatcher selects from: its
 * three words are 0x800C18D8, 0x800C18E0 and 0x800C18E8, the inert handlers of
 * this overlay. They are the last words of the pointer run the payload keeps
 * from 0x800C19BC to its end at 0x800C19F8, so this declaration binds only the
 * word the scaled index is added to and the run stays one span.
 */
extern Boss02916Handler D_800C19EC[]; /* @source 0x800C19EC @kind data */

/* @source 0x800C18A0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C19EC
 * handler table, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled table
 * load, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. No word of this payload names this function's address, so it is an
 * overlay entry reached from outside the payload. Same C shape as the exact
 * indexed dispatchers at 0x800C1AF0 of BIN/BOSS/BOSS028.EMI#16, 0x800C18B8 of
 * BIN/BOSS/BOSS050.EMI#16 and 0x800C1C88 of BIN/BOSS/BOSS008.EMI#16; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02916_800C18A0(u8 index) {
  D_800C19EC[index]();
}
