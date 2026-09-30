#include "bof3/bof3.h"

extern void (*D_801F581C[])(void);

/* @source 0x801F41D0
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the second local handler table
 * based at 0x801F581C. That table's row in the shipped image reads, in order,
 * 0x801F4214, 0x801F4288, 0x801F4310, 0x801F4388, 0x801F4400, 0x801F44C0,
 * 0x801F453C, 0x801F45A0, 0x801F4624, 0x801F46AC and 0x801F46CC, and the word
 * after it at 0x801F5848 is 1 rather than a code pointer (raw .word row inside
 * the trailing asm blob of this overlay); three of those entries are the
 * already-exact same-target handlers
 * clearWorkFlagsWhenNoCallbackRanWorld03Area13413_801F4310,
 * invokeHelperWorld03Area13413_801F453C and
 * invokeHelperWorld03Area13413_801F46CC. The sibling at 0x801F3E0C is the same
 * dispatch through the first local handler table 0x801F5804, and the
 * already-exact sibling dispatchStateByte1Table57a4World03Area13413_801F3560 is
 * the same dispatch through the table based at 0x801F57A4. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table581cWorld03Area13413_801F41D0(void) {
  D_801F581C[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
