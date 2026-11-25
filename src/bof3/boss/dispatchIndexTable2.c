#include "bof3/bof3.h"

typedef void (*Boss03016Handler)(void);

/*
 * Handler table at 0x800C1B48, the three-word run this dispatcher selects from:
 * 0x800C19C4, 0x800C19CC and 0x800C19D4, the inert handlers of this overlay.
 * The words are the tail of the pointer run the Splat `textbin` row labels
 * 0x800C1AB4 and they end where the payload ends, at 0x800C1B54, so this
 * declaration binds only the word the scaled index is added to and the run
 * stays one span. The step setup at 0x800C1934 publishes this table into +0xEC
 * of the current enemy work record, beside this function's own address in +0xE4.
 */
extern Boss03016Handler D_800C1B48[]; /* @source 0x800C1B48 @kind data */

/* @source 0x800C198C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1B48
 * handler table, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled table
 * load, and the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call. The step setup at 0x800C1934 stores this function's address into +0xE4
 * of the current enemy work record loaded from 0x801EB4E8, so the state machine
 * enters it through that stored pointer word; no jal immediate in this payload
 * reaches it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: the 56 payload bytes at 0x800C198C differ from the reviewed
 * exact span at 0x800C18A0 of this payload in one byte, the low byte of the
 * `lw` that addresses the table (0x48 here, 0x0C there), so the statement
 * order, operand forms and declaration style come from that exact sibling
 * while the table binding is written from this span's own load word.
 */
void dispatchIndexTable2(u8 index) {
  D_800C1B48[index]();
}
