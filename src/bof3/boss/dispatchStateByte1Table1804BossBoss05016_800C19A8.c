#include "bof3/bof3.h"

typedef void (*Boss05016Handler)(void);

typedef struct Boss05016DispatchTable {
  Boss05016Handler handlers[3];
} Boss05016DispatchTable;

/*
 * Handler table at 0x800C1804, the three pointers this dispatcher copies into
 * its frame: entry 0 = 0x800C1A08, entry 1 = 0x800C1A84 and entry 2 =
 * 0x800C1AEC, the three step handlers that follow it in this overlay. The run
 * occupies the first twelve bytes of the span mapped at 0x800C1804, whose code
 * proper starts at 0x800C1810, so this declaration binds the address-anchored
 * map symbol that already covers that span.
 */
extern Boss05016DispatchTable func_800C1804; /* @source 0x800C1804 @kind data */

/* @source 0x800C19A8
 * @behavior Overlay step dispatch on work-record byte 1: copies the three
 * handler pointers of the table at 0x800C1804 (0x800C1A08, 0x800C1A84,
 * 0x800C1AEC) into a local frame table, then calls the entry selected by
 * unsigned byte 1 of the record published at scratchpad pointer cell
 * 0x1F800044 with no arguments. The record-pointer load precedes frame
 * creation, the index is not bounded before the scaled load, and the 0x28-byte
 * frame keeps $ra at 0x20 with the copied words at 0x10 through 0x18. It reads
 * no other state and writes none; the indirect call sets no argument register
 * and the epilogue leaves the selected handler's $v0 untouched. Same
 * 24-instruction, 96-byte body as the cross-target exact sibling
 * dispatchWorkByte1HandlerBmagicMagic05703_801EF02C, whose three-entry table
 * copy, frame, load/store order, delay slots and zero-argument indirect call
 * are instruction-for-instruction the same; only the table the lui/addiu pair
 * materializes differs, so that sibling supplies the shape and not the bytes.
 * The four-entry form of the same overlay-local dispatcher is the exact
 * sibling dispatchStateByte1Table1804BossBoss05116_800C1AA8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table1804BossBoss05016_800C19A8(void) {
  Boss05016DispatchTable handlers;

  handlers = func_800C1804;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
