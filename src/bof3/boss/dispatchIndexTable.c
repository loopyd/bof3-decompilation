#include "bof3/bof3.h"

typedef void (*Boss03016Handler)(void);

/*
 * Handler table at 0x800C1B0C, the four-word run this dispatcher selects from:
 * entries 0 through 2 are the inert handlers of this overlay at 0x800C18D8,
 * 0x800C18E0 and 0x800C18E8 and entry 3 is the sibling publisher at
 * 0x800C1934. It is an interior word of the pointer run the Splat `textbin`
 * row labels 0x800C1AB4, so this declaration binds only the word the scaled
 * index is added to and the run stays one span.
 */
extern Boss03016Handler D_800C1B0C[]; /* @source 0x800C1B0C @kind data */

/* @source 0x800C18A0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1B0C
 * handler table, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled table
 * load, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. The step setup at 0x800C1848 publishes this address into +0xE4 of the
 * enemy work record loaded from 0x801EB4E8, so the state machine enters it
 * through that stored pointer word; no jal immediate in this payload reaches
 * it. Same C shape as the exact indexed dispatcher at 0x800C18A0 of
 * BIN/BOSS/BOSS029.EMI#16, whose body differs only in that table word
 * (0x800C19EC there), and as the byte-equal sibling at 0x800C198C of this
 * payload, whose body differs only in its own table word (0x800C1B48).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexTable(u8 index) {
  D_800C1B0C[index]();
}
