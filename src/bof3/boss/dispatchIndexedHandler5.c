#include "bof3/bof3.h"

typedef void (*Boss04016Handler)(void);

/*
 * Handler table at 0x800C1B9C, the table this dispatcher selects from: entry 0 =
 * 0x800C19DC, entry 1 = 0x800C19E4 and entry 2 = 0x800C19EC, the three inert
 * handlers that close this overlay's code. The base sits inside the pointer run
 * the payload keeps from 0x800C1B1C to its end, reviewed as the textbin row at
 * 0x800C1B1C, so this declaration binds only the word the scaled index is added
 * to and the run stays one row.
 */
extern Boss04016Handler D_800C1B9C[]; /* @source 0x800C1B9C @kind data */

/* @source 0x800C19A4
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1B9C handler
 * table, so the selected handler's result stays in the return register. This
 * span was byte-identical to the body of the sibling dispatcher
 * dispatchIndexedHandler4, which selects from the
 * 0x800C1B60 table and gated exact when this body was written; only the low half
 * of the table load word differs. The same span also sat byte-identical at
 * 0x800C19A4 of BIN/BOSS/BOSS049.EMI#16, whose row was still an asm row, so that
 * payload supplied no source and only confirmed the span. The installer at
 * 0x800C1940 stores this function's address into offset 0xE4 of the record it
 * reaches through D_801EB4E8, the slot the earlier installer at 0x800C1848
 * filled with the 0x800C18AC dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandler5(u8 index) {
  D_800C1B9C[index]();
}
