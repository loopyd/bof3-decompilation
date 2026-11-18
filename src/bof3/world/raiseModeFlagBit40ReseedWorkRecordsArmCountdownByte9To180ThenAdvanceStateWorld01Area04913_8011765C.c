#include "bof3/bof3.h"

/* Work-table seed of this overlay, still on its asm row: it publishes the
 * 0x800E4800 work table in D_801181DC and rewrites all sixteen 0x28-byte
 * records of it from the work object at the scratchpad pointer cell, giving each
 * a rand velocity pair, a set claim byte and a cleared step byte. */
void func_801172D0(void);

/* World/mode flag halfword of the shared main executable. This body raises bit
 * 0x40 of it; no lift of that executable names the bit yet, so the halfword and
 * the bit stay address-anchored.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address with `lui` plus an offset load, and it loads the cell once before the
 * object's countdown store and once after it, so the source reads the named
 * symbol twice.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x8011765C
 * @behavior Step handler 4 of the thirteen in-image pointers the Splat textbin
 * row D_8011818C places: word 4 at 0x8011819C, the base
 * dispatchWorkByte1Table819cThenRunRecordPassWorld01Area04913_80117610 indexes
 * from the work object's byte at offset 1. It raises bit 0x40 of the shared
 * world/mode flag halfword 0x80146258, runs the work-table seed func_801172D0,
 * which republishes the 0x800E4800 table in D_801181DC and rewrites all sixteen
 * of its records from the work object, and then arms that object's countdown
 * byte at offset 9 to 0xB4, the 180 the next handler
 * func_801176B8 counts down before it re-arms the byte to 0x40. Finally it
 * re-reads the pointer cell and increments the object byte at 1 by one, so the
 * next state of the overlay runs from the following frame. The flag halfword is
 * read before the frame is created, the pointer cell is loaded once per store
 * group because a store through the record pointer can alias it, the 0x18-byte
 * frame only keeps $ra at 0x10 across the single call, and the slots after the
 * second cell load and after the countdown byte load stay nops. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseModeFlagBit40ReseedWorkRecordsArmCountdownByte9To180ThenAdvanceStateWorld01Area04913_8011765C(
    void) {
  u8* work;

  D_80146258 |= 0x40;
  func_801172D0();
  work = D_1F800044;
  work[9] = 0xB4;
  work = D_1F800044;
  work[1] = work[1] + 1;
}
