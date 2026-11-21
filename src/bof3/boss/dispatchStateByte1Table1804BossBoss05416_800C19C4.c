#include "bof3/bof3.h"

typedef void (*Boss05416StepHandler)(void);

typedef struct Boss05416StepTable {
  Boss05416StepHandler handlers[10];
} Boss05416StepTable;

/*
 * In-image code pointer run at 0x800C1804: the thirteen words 0x800C1804
 * through 0x800C1834, the step handlers of this overlay, whose whole span the
 * reviewed `textbin` row at 0x800C1804 carries (entry 0 = 0x800C1A54,
 * entry 1 = 0x800C1AB4, entry 2 = 0x800C1B40, entry 3 = 0x800C1BA8,
 * entry 4 = 0x800C1C18, entry 5 = 0x800C1BA8, entry 6 = 0x800C1C18,
 * entry 7 = 0x800C1BA8, entry 8 = 0x800C1C18, entry 9 = 0x800C1CFC,
 * entry 10 = 0x800C1E0C, entry 11 = 0x800C1FA8, entry 12 = 0x800C2030). This
 * dispatcher copies the ten leading words only, so this declaration references
 * that prefix of the reviewed row and not the three words that follow it.
 */
extern Boss05416StepTable func_800C1804; /* @source 0x800C1804 @kind data */

/* @source 0x800C19C4
 * @behavior Overlay step dispatch on work-record byte 1 from a frame copy of
 * this payload's own handler run: copies the ten leading words of the run at
 * 0x800C1804 (0x800C1A54, 0x800C1AB4, 0x800C1B40, 0x800C1BA8, 0x800C1C18,
 * 0x800C1BA8, 0x800C1C18, 0x800C1BA8, 0x800C1C18, 0x800C1CFC) into a local
 * 0x28-byte frame table, then calls the entry selected by unsigned byte 1 of
 * the record published at the scratchpad pointer cell 0x1F800044 with no
 * arguments. At ten words that copy is a four-word loop over the leading 0x20
 * bytes plus a two-word tail; the 0x40-byte frame keeps $ra at 0x38 with the
 * copied words at 0x10 through 0x34, and the record-pointer load follows the
 * copy. The index is not bounded before the scaled load, the indirect call
 * sets no argument register, and the epilogue restores $ra and $sp without
 * touching $v0, so this function returns whatever the selected handler
 * returns. It reads no other state and writes none. The payload holds neither
 * a jal to 0x800C19C4 nor a pointer word naming it, so the overlay loader that
 * steps the work record is its caller and it takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table1804BossBoss05416_800C19C4(void) {
  Boss05416StepTable handlers;

  handlers = func_800C1804;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
