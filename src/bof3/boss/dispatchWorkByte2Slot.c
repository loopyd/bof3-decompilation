#include "bof3/bof3.h"

typedef void (*Boss03616StepHandler)(void);

typedef struct Boss03616StepTable {
  Boss03616StepHandler handlers[3];
} Boss03616StepTable;

/*
 * Three-word in-image code pointer run at 0x800C1804: entry 0 = 0x800C1F44,
 * entry 1 = 0x800C1FA4, entry 2 = 0x800C2000, the three step handlers that
 * follow this dispatcher in the payload. The reviewed `textbin` row D_800C1804
 * covers the twelve bytes 0x800C1804 through 0x800C180F and ends where this
 * payload's dispatcher row at 0x800C1810 begins, so the run is placed by the
 * reviewed Splat layout and this declaration only references its prefix.
 */
extern Boss03616StepTable D_800C1804; /* @source 0x800C1804 @kind data */

/* @source 0x800C1EE4
 * @behavior Overlay step dispatch on work-record byte 2: copies the three
 * handler pointers of the run at 0x800C1804 (0x800C1F44, 0x800C1FA4,
 * 0x800C2000) into a local frame table, then calls the entry selected by
 * unsigned byte 2 of the record published at the scratchpad pointer cell
 * 0x1F800044 with no arguments. The record-pointer load precedes frame
 * creation, the index is not bounded before the scaled load, and the 0x28-byte
 * frame keeps $ra at 0x20 with the three copied words at 0x10 through 0x18. It
 * reads no other state and writes none, the indirect call sets no argument
 * register, and the epilogue leaves the selected handler's $v0 untouched, so
 * this function returns whatever that handler returns. The payload holds no jal
 * naming 0x800C1EE4; the single in-image pointer word that names it is
 * 0x800C20BC, the last word of the reviewed run D_800C2020 and the base the
 * byte-1 dispatcher at 0x800C1EA0 indexes. Its 24-instruction, 96-byte body
 * measured equal to the previously lifted sibling
 * dispatchStateByte1Table1804BossBoss05016_800C19A8 in every word but the
 * selector's `lbu` immediate, which is byte 2 here and byte 1 there; that
 * sibling supplied the statement order, operand forms and declaration style,
 * while this span's own bytes supplied the selector slot and the copied run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Slot(void) {
  Boss03616StepTable handlers;

  handlers = D_800C1804;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
