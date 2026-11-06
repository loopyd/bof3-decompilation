#include "bof3/bof3.h"

typedef void (*Boss03216Handler)(void);

/*
 * Handler table at 0x800C1C88, the table this dispatcher selects from: its
 * three words are 0x800C19B0, 0x800C19B8 and 0x800C19C0, the inert handlers of
 * this overlay. They are the last words of the pointer run the payload keeps
 * from 0x800C1C40 to 0x800C1C94 inside the Splat asm span at 0x800C1C2C, which
 * disassembles as a word run. The generated undefined_syms_auto.txt row for
 * D_800C1C88 already binds the word the scaled index is added to, so this
 * declaration needs no map row.
 */
extern Boss03216Handler D_800C1C88[]; /* @source 0x800C1C88 @kind data */

/* @source 0x800C1978
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1C88
 * handler table, so the selected handler's result stays in the return
 * register. All three table words are inert handlers of this overlay, so an
 * index in range returns without touching any state. No instruction in this
 * payload references this function's address; the main program reaches it
 * through its overlay entry point at 0x800C18F0. Same C shape as the exact
 * indexed dispatchers at 0x800C19C8 of BIN/BOSS/BOSS002.EMI#16 and at
 * 0x800C1998 of BIN/BOSS/BOSS014.EMI#16; only the table address in the load
 * word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03216_800C1978(u8 index) {
  D_800C1C88[index]();
}
