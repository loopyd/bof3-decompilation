#include "bof3/bof3.h"

typedef void (*Boss05216Handler)(void);

/*
 * Handler table at 0x800C1F98, the table this dispatcher selects from: its four
 * words are 0x800C19BC, 0x800C19C4, 0x800C19CC and 0x800C1D98, the two inert
 * handlers that open the run, the handler at 0x800C19CC and this overlay's step
 * dispatcher. They are the last words of the pointer run the payload keeps from
 * 0x800C1F50 to its end at 0x800C1FA4, so this declaration binds only the word
 * the scaled index is added to and the run stays one span.
 */
extern Boss05216Handler D_800C1F98[]; /* @source 0x800C1F98 @kind data */

/* @source 0x800C1984
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1F98 handler
 * table, so the selected handler's result stays in the return register. The
 * original does not bound the index before the scaled table load, and the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call. The overlay
 * init at 0x800C1854 stores this address at offset 0xE4 of the record reached
 * through the 0x801EB4E8 cell, so the main program reaches it as a registered
 * callback slot rather than through a jal site of this payload. Same C shape as
 * the exact indexed dispatcher at 0x800C18A0 of BIN/BOSS/BOSS029.EMI#16; only
 * the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler36(u8 index) {
  D_800C1F98[index]();
}
