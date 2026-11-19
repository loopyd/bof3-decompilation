#include "bof3/bof3.h"

typedef void (*Boss03113Handler)(void);

/*
 * Handler table at 0x800C1B14, the table this dispatcher selects from: its three
 * words are 0x800C19B0, 0x800C19B8 and 0x800C19C0, the inert handlers of this
 * overlay. They are the last three words of the pointer run the payload keeps to
 * its end at 0x800C1B20, so this declaration binds only the word the scaled
 * index is added to and the run stays one span.
 */
extern Boss03113Handler D_800C1B14[]; /* @source 0x800C1B14 @kind data */

/* @source 0x800C1978
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1B14 handler
 * table, so the selected handler's result stays in the return register. The
 * original does not bound the index before the scaled table load, and the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call. No word of
 * this payload names this function's address, so it is an overlay entry reached
 * from outside the payload. Same C shape as the exact indexed dispatchers at
 * 0x800C18A0 of BIN/BOSS/BOSS029.EMI#16 and 0x800C1A68 of BIN/BOSS/BOSS025.EMI#13;
 * against the former the fourteen instructions are byte-identical except the
 * %lo immediate of the table load, which names 0x800C19EC there and 0x800C1B14
 * here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03113_800C1978(u8 index) {
  D_800C1B14[index]();
}
