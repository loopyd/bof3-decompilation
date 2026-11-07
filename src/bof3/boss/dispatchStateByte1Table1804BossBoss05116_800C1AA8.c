#include "bof3/bof3.h"

typedef void (*BossBoss05116Handler)(void);

typedef struct BossBoss05116DispatchTable {
  BossBoss05116Handler handlers[4];
} BossBoss05116DispatchTable;

/*
 * Handler table at 0x800C1804, the four pointers this dispatcher copies into
 * its frame: entry 0 = 0x800C1B14, entry 1 = 0x800C1BF0, entry 2 = 0x800C1CE4
 * and entry 3 = 0x800C1D7C, the four step handlers that follow it in this
 * overlay. The run occupies the first sixteen bytes of the span mapped at
 * 0x800C1804, whose code proper starts at 0x800C1814, so this declaration
 * binds the address-anchored map symbol that already covers that span.
 */
extern BossBoss05116DispatchTable func_800C1804; /* @source 0x800C1804 @kind data */

/* @source 0x800C1AA8
 * @behavior Overlay step dispatch on work-record byte 1: copies the four
 * handler pointers of the table at 0x800C1804 (0x800C1B14, 0x800C1BF0,
 * 0x800C1CE4, 0x800C1D7C) into a local frame table, then calls the entry
 * selected by unsigned byte 1 of the record published at scratchpad pointer
 * cell 0x1F800044 with no arguments. The record-pointer load precedes frame
 * creation, the index is not bounded before the scaled load, and the
 * 0x28-byte frame keeps $ra at 0x20 with the copied words at 0x10 through
 * 0x1C. It reads no other state and writes none; the indirect call sets no
 * argument register and the epilogue leaves the selected handler's $v0
 * untouched. Same 27-instruction, 108-byte body as the cross-target exact
 * sibling dispatchWorkByte1HandlerBmagicMagic01500_801EECA0, whose frame,
 * load/store order, delay slots and zero-argument indirect call are
 * instruction-for-instruction the same; only the table the lui/addiu pair
 * materializes differs, so that sibling supplies the shape and not the bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table1804BossBoss05116_800C1AA8(void) {
  BossBoss05116DispatchTable handlers;

  handlers = func_800C1804;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
