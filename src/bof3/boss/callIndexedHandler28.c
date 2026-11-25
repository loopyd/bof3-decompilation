#include "bof3/bof3.h"

typedef void (*Boss02213Handler)(void);

/*
 * Handler table at 0x800C1B74, the table this dispatcher selects from: its three
 * words are 0x800C19F0, 0x800C19F8 and 0x800C1A00, the inert handlers of this
 * overlay. They are the last three words of the payload, at the end of the code
 * pointer run that starts at 0x800C1B2C inside this overlay's 0x800C1B18 data
 * row, so this declaration binds only the word the scaled index is added to and
 * the run stays one span.
 */
extern Boss02213Handler D_800C1B74[]; /* @source 0x800C1B74 @kind data */

/* @source 0x800C19B8
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1B74 handler
 * table, so the selected handler's result stays in the return register and the
 * handler sees the scaled index in $a0. The original does not bound the index
 * before the scaled table load, and the 0x18-byte frame only keeps $ra at 0x10
 * across the indirect call. No word of this payload names this function's
 * address and no jal reaches it, so it is an overlay entry reached from outside
 * the payload. The span is byte-identical to the exact indexed dispatcher at
 * 0x800C18A0 of BIN/BOSS/BOSS014.EMI#16, all fourteen instructions including
 * the table literal, so no word here is written from a shape argument; against
 * the exact indexed dispatcher at 0x800C1998 of that same overlay only the %lo
 * immediate of the table load differs, which names 0x800C1BB0 there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler28(u8 index) {
  D_800C1B74[index]();
}
